/*
 * player.c
 *
 *  Created on: 29 lip 2020
 *      Author: dbank
 */

#include <stdio.h>
#include <string.h>
#include "ff.h"
#include "play_wav.h"
#include "play_mp3.h"
#include "play_aac.h"
#include "player.h"

/* zmienne do kontroli odczytu plików */

int nRead, bytes_left;
int err, outOfData/* brak danych w buforze odczytu pliku*/, eofReached/* end of file*/ , nFrames;
//static uint32_t  bytes_leftBeforeDecoding;
uint32_t  offset;								//zmienne do kontroli zapełnienia buforów dekodujących audio, mówi, gdzie funkcja dekodująca skończyła dekodownaie, ile danych pozostało nieodczytanych w buforze pliku
unsigned char *read_ptr;
//static uint32_t bytes_to_read;							// ilość danych do przeczytania, aby uzupełnic bufor danych wejściowych
/* zmienne do kontroli odczytu plików */


/* bufory do odczytu plików, dekodowani audio i odtwarzania dźwięku*/
unsigned char inBuffer[READ_BUFFER_SIZE];			//bufor do którego są czytane pliki
uint16_t decBuffer[DECODED_AUDIO_FRAME_SIZE];	//bufor do którego są dekodowane dane audio
uint16_t dmaBuffer[DMA_BUFFER_SIZE];			//bufor do którego są kopiowane dane audio i z którego I2S gra muzykę
/* bufory do odczytu plików, dekodowani audio i odtwarzania dźwięku*/


/* zmienne do kontroli odtwarzania */
volatile enum DMABufferStateTypeDef DMABufferState;	//zmienna za pomocą, której kontrolowany jest stan buforów DMA
volatile enum PlayerStateTypeDef PlayerState;		//zmienna za pomocą której kontrolowany jest stan pdtwarzacza
/* zmienne do kontroli odtwarzania */

/* zmienne zewnętrzne */
extern I2S_HandleTypeDef hi2s2;			//zmienna kontrolująca interfejs I2S zadeklarowana w main.c
/* zmienne zewnętrzne */

/**
  * @brief 	Główna funkcja odtwarzająca pliki audio znajdujące się w katalogu.
  * 		Wyszukuje pliki audio, rozpoznaje format i odtwarza.
  *
  * @note
  *
  * @param	const char* path 	- ścieżka dostępu do katalogu, w którym znajdują się pliki audio
  * 		unsigned char seek	- numer pliku, od którego ma zacząć odtwarzanie //TODO: jeszcze nie wdrożono obsługi tego
  *
  * @retval	FRESULT				- //TODO: bez sensu żeby zwracało ta wartość wymysleć inną.
  *
  */

FRESULT play_directory (const char* path) { //, unsigned char seek)	////TODO: dorobić obsługę "seek"
//FRESULT play_directory (const char* path, unsigned char seek) {
	FRESULT errFS;	//FRESULT res;
	FILINFO fno; // wskaźnik do katalogu w którym ma szukac muzyki
	DIR dir;
	FIL  fno_audio; // wskaźnik do granego pliku audio
	//char *fn; /* This function is assuming non-Unicode cfg. */

#if _USE_LFN
//	static char lfn[_MAX_LFN + 1];
//	fno.lfname = lfn;
//	fno.lfsize = sizeof(lfn);
	#error "_USE_LFN not supported"
#endif

	errFS = f_opendir(&dir, path); /* Open the directory */
	HAL_Delay(10);  // File access latency time - nie wiem raz potrzebne raz nie
	if (errFS == FR_OK) {

		do{
			errFS = f_readdir(&dir, &fno); /* Read a directory item */

			if (errFS != FR_OK || fno.fname[0] == 0) break; /* Break on error or end of dir */
			if (fno.fname[0] == '.') continue; /* Ignore dot entry */
//#if _USE_LFN
//			//fn = *fno.lfname ? fno.lfname : fno.fname;
//#else
//			fn = fno.fname;
//#endif
			if (fno.fattrib & AM_DIR) { /* It is a directory */

			} else { /* It is a file. */

				{
					char filePath[16];//17
					memset(&filePath, 0, 17);
					memcpy(&filePath,path,3);
					//memcpy(&filePath[3],fn,13);
					memcpy(&filePath[3],fno.fname,13);

					//OTWIERANIE PLIKU DO ODCZYTU
					errFS = f_open(&fno_audio, &filePath/*fn*/, FA_OPEN_EXISTING|FA_READ);
				}



				Player_reset();

				if (errFS!= FR_OK)
				{
					printf("PLAYER: Failed to open file \"%s\" for reading, err=%d\r\n", fno.fname/*fn*/, errFS);
				}
				else
				{
					//Jeśli plik został poprawnie otworzony, to zaczyna się jego odtwarzanie
					switch (get_filetype(fno.fname/*fn*/)){
					case UNKNOWN:
						printf ("PLAYER: unknown file type: %s\r\n",fno.fname/*fn*/);
						break;
					case WAV:
						printf ("\r\nPLAYER: playing WAV: %s\r\n",fno.fname/*fn*/);
						WavProcess(&fno_audio);
						break;
					case MP3:
						printf ("\r\nPLAYER: playing MP3: %s\r\n",fno.fname/*fn*/);
						Mp3Process(&fno_audio);
						break;
					case AAC:
						printf ("\r\nPLAYER: playing AAC: %s\r\n",fno.fname/*fn*/);
						AacProcess(&fno_audio, AACdec);		//TODO: Jak wyjmuję nośnik danych to zawiesza odtwarzanie. Zrobić porcedurę WatchDogTimer i Error handler
						break;
					case MP4:
					case MP2:
					case M4A:
					case WMA:
					case M4P:
					case OGG:
						printf ("PLAYER: audio file not supported yet: %s\r\n",fno.fname/*fn*/);
						break;
					}
					Player_reset();
					if (get_filetype(fno.fname/*fn*/)!=UNKNOWN)
						printf ("PLAYER: stop playing: %s\r\n\r\n",fno.fname/*fn*/);
				}
				f_close(&fno_audio);
			}
		}while (1);
	}
	// ZAMYKANIE dostępu do katalogu
	f_closedir(&dir);
	printf ("PLAYER: close access to dir: %s\r\n",path);
	return errFS;
}


//Tabela stałych wpływająca na jakośc i2s określona w Audio frequency precission (for PLLM VCO ...)
static const uint32_t	I2SFreq[8]	= {8000,	11025,	16000,	22050,	32000,	44100,	48000,	96000	};	//wartości w /* */ są wartościami ze skryptu z internetu
static const uint32_t	I2SPLLN[8]	= {256,		429,	213,	429,	426,	271,	258,	344		};	//wartości w /* */ są wartościami ze skryptu z internetu
static const uint32_t	I2SPLLR[8]	= {5,		4,		4,		4,		4,		6,		3,		2		};
//static const uint8_t	I2SDIV[8]	= {12,		19,		6,		9,		6,		2,		3,		3		};
//static const uint8_t 	I2SODD[8]	= {1,		1,		1,		1,		1,		0,		1,		1		};	//MOŻE PRZYJMOWAĆ WARTOŚĆ 0 / 1

void playIfReady(uint16_t* dmaBuffer2Play, uint16_t I2SdmaBufferSize, uint32_t sampleRate)
{
	if ((PlayerState == Player_stopped)&& DMABufferState == DMABuffer_Full)		//Jeśłi bufor DMA jest pełny i player zatrzymany to rozpoczyna odtwarzanie
																				// jeśli nie to rozpoczyna napełnianie w petli buforów DMA i dopiero wtedy odtwarzanie
	{
		audioI2S_pllClockConfig(sampleRate);									//dopasowuje ustawienia zegarów do częstotliwości pliku audio
		I2S3_freqUpdate(sampleRate);											//dopasowuje ustawienia i2s do częstotliwości pliku audio
		if (HAL_I2S_Transmit_DMA(&hi2s2, dmaBuffer2Play, I2SdmaBufferSize)==HAL_OK)
		{
			printf("PLAYER: playing.\r\n");
			PlayerState = Player_playing;
		}
		else
		{
			printf("PLAYER: problem with player settings.\r\n");
			PlayerState = Player_error;
		}
	}
}

static void audioI2S_pllClockConfig(uint32_t audioFreq)
{
  RCC_PeriphCLKInitTypeDef rccclkinit;
  uint8_t index = 0, freqindex = 0xFF;

  for(index = 0; index < 8; index++)
  {
    if(I2SFreq[index] == audioFreq)
    {
      freqindex = index;
    }
  }
  /* Enable PLLI2S clock */
  HAL_RCCEx_GetPeriphCLKConfig(&rccclkinit);
  /* PLLI2S_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  if ((freqindex & 0x7) == 0)
  {
    /* I2S clock config
    PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) × (PLLI2SN/PLLM)
    I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR */
    rccclkinit.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    rccclkinit.PLLI2S.PLLI2SN = I2SPLLN[freqindex];
    rccclkinit.PLLI2S.PLLI2SR = I2SPLLR[freqindex];
    HAL_RCCEx_PeriphCLKConfig(&rccclkinit);
  }
  else
  {
    /* I2S clock config
    PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) × (PLLI2SN/PLLM)
    I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR */
    rccclkinit.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    rccclkinit.PLLI2S.PLLI2SN = 258;
    rccclkinit.PLLI2S.PLLI2SR = 3;
    HAL_RCCEx_PeriphCLKConfig(&rccclkinit);
  }
}

static void I2S3_freqUpdate(uint32_t AudioFreq)
{

  HAL_I2S_DeInit(&hi2s2);

  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s2.Init.AudioFreq = AudioFreq;
  hi2s2.Init.CPOL = I2S_CPOL_HIGH;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;


  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
}

void Player_reset(void)
{
	//stop I2S & DMA
	HAL_I2S_DMAStop(&hi2s2);
	HAL_I2S_DeInit(&hi2s2);

	//reset file access variable(s)
	bytes_left=0;
	//bytes_leftBeforeDecoding=0;
	offset=0;
	read_ptr = inBuffer;
	outOfData = 0;
	eofReached = 0;
	nRead = 0;
	nFrames = 0;
	err= 0; //ERR_MP3_NONE

	//reset buffer(s)
	memset(inBuffer,NULL,sizeof(inBuffer));
	memset(decBuffer,NULL,sizeof(decBuffer));
	memset(dmaBuffer,NULL,sizeof(dmaBuffer));

	//reset player & DMA variable(s)
	DMABufferState = DMABuffer_1stHalf_empty | DMABuffer_2ndHalf_empty;
	PlayerState = Player_stopped;
}


int FillReadBuffer(unsigned char *readBuf, unsigned char *readPtr, int bufSize, int bytesLeft, FILE *infile)
{
	UINT nRead;
	uint32_t bytes_to_read = bufSize - bytesLeft;//READ_BUFFER_SIZE to stary  mp3buf_size, ile danych można jeszcze wrzucić do budofa

	memcpy(readBuf, readPtr, bytesLeft);	//kopiuje pozostałą część bufora na sam przód
	f_read(infile, readBuf + bytesLeft, bytes_to_read, &nRead);
	if (nRead < bytes_to_read) {
		memset(readBuf + bytesLeft + nRead, NULL, bytes_to_read - nRead);
	}
	//return (uint32_t) bytesLeft + (uint32_t) nRead;
	return (uint32_t) nRead;
}



void fill_DMAbuffer(uint16_t* destinationBuffer, uint16_t* sourceBuffer, uint32_t destinationBufferSize)
{

// !!! Kawałek kodu do debugowania buforów  i ich wielkości wykasować w ostatecznej wersji programy !!!!/
//	uint32_t	inbufsize = sizeof(unsigned char)*READ_BUFFER_SIZE;							//bufor wejściowy plików 				= 4096
//
//	uint32_t 	decbufsize =	sizeof(uint16_t)	*DECODED_AUDIO_FRAME_SIZE;			//bufor dekodowania danych 				= 2304
//	uint32_t	decbufWav  =	sizeof(uint16_t)	*DECODED_AUDIO_FRAME_SIZE_WAV;		//bufor dekodowania danych - WAV		= 2000
//	uint32_t	toCopy =		destinationBufferSize;										//ilość danych do przekopiowania		= 2000 dla WAV
//
//
//	uint32_t 	dmabufsize =	sizeof(dmaBuffer);											//bufor I2S								= 9216
//	uint32_t	dmabufWAv =		sizeof(uint16_t)*DMA_BUFFER_SIZE_WAV;						//bufor i2s potrzebny dla WAV			= 4000 dla WAV


	switch (DMABufferState)
	{
	//case (DMABuffer_Full):	//tego case'a nie może być, bo wtedy podczas odtwarzania są trzaski
	case (DMABuffer_1stHalf_empty):
		memcpy(destinationBuffer, sourceBuffer, destinationBufferSize);		//kopiuje rozkodowane dane do 1st połowy bufora DMA
		DMABufferState &= 0b11111110;
		break;
	case (DMABuffer_2ndHalf_empty):
		memcpy(destinationBuffer+destinationBufferSize/2, sourceBuffer, destinationBufferSize);		//kopiuje rozkodowane dane do 2nd połowy bufora DMA
		DMABufferState &= 0b11111101;
		break;
	case (DMABuffer_1stHalf_empty | DMABuffer_2ndHalf_empty):
		HAL_I2S_DMAStop(&hi2s2);
		printf("PLAYER: Both halfs of DMA buffer empty. Buffer synchronisation issue.\r\n");
		printf("PLAYER: DMA transmission stopped.\r\n");
		memcpy(destinationBuffer, sourceBuffer, destinationBufferSize);		//kopiuje rozkodowane dane do 1st połowy bufora DMA
		DMABufferState &= 0b11111110;
		PlayerState = Player_stopped;
		break;
	}
}

void convert_mono2stereo(uint16_t* buffer, uint32_t buffer_size)
{
	uint32_t c = ((buffer_size/2)-1);
	do {
		*(buffer+c*2) = *(buffer+c*2+1)= *(buffer+c);
		if (c >0)
			c--;
		else
		{
			*(buffer+c*2) = *(buffer+c*2+1)= *(buffer+c);
			break;
		}
	}while (1);	//}while (c >=0);
}

static enum audioFileTypeDef get_filetype(char * filename)
{
	char *extension;

	extension = strrchr(filename, '.') + 1;

	if(strncasecmp(extension, "MP2", 3) == 0) {
		return MP2;
	} else if (strncasecmp(extension, "MP3", 3) == 0) {
		return MP3;
	} else if (strncasecmp(extension, "MP4", 3) == 0) {
		return MP4;
	} else if (strncasecmp(extension, "M4A", 3) == 0) {
			return M4A;
	} else if (strncasecmp(extension, "WMA", 3) == 0) {
			return WMA;
	} else if (strncasecmp(extension, "M4P", 3) == 0) {
			return M4P;
	} else if (strncasecmp(extension, "OGG", 3) == 0) {
			return OGG;
	} else if (strncasecmp(extension, "AAC", 3) == 0) {
		return AAC;
	} else if (strncasecmp(extension, "WAV", 3) == 0 ||
	           strncasecmp(extension, "RAW", 3) == 0) {
		return WAV;
	} else {
		return UNKNOWN;
	}
}

void printf_audio_information(char* decoderName, unsigned long samprate, unsigned short bitsPerSample, unsigned short nChans)
{
	printf("%sDec: %ld Hz %d Bit %d Channel(s)\r\n",
			decoderName, samprate, bitsPerSample, nChans);
}

void wait4_I2S_DMA_SyncSignal(void)		//TODO: static???
{
	//bool wasDisplayed = FALSE;
	//const char notification [] = "PLAYER: waiting4DMA buffer sync signal\r\n\0";
	//u_int i =0;
	while (DMABufferState==DMABuffer_Full)
	{
//		if (wasDisplayed==FALSE)
//		{
//			printf("%c", notification[i]);		//ten printf nie może być czytany z EEPROMU
//			if (i==sizeof(notification))
//			{
//				//i=0;
//				wasDisplayed = TRUE;
//			}
//			else
//				i++;
//		}
	};
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
  /* Prevent unused argument(s) compilation warning */
	if(hi2s->Instance == SPI2)
		DMABufferState |= DMABuffer_1stHalf_empty;
//	printf("h\r\n");
//  UNUSED(hi2s);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
  /* Prevent unused argument(s) compilation warning */
	if(hi2s->Instance == SPI2)
		DMABufferState |= DMABuffer_2ndHalf_empty;
//	printf("f\r\n");
//  UNUSED(hi2s);
}
