/*
 * play_wav.c
 *
 *  Created on: 29 lip 2020
 *      Author: dbank
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ff.h"
#include "play_wav.h"
#include "player.h"




/* zmienne likalne */
static struct WAV_HeaderTypeDef 	WAV_header;		//zmienna przechowujące informacje na temat nagłówka pliku typu WAVE
/* zmienne likalne */

/* zmienne zewnętrzne */
//extern UINT br;						//zmienne do kontroli odczytu plików (używana w play_mp.c, play_wav.c, itp), zadeklarowana w player.c
extern uint32_t offset;
//extern uint32_t bytes_left;

extern int nRead, bytes_left, nFrames;
extern unsigned char *read_ptr;

/* bufory do odczytu plików, dekodowani audio i odtwarzania dźwięku*/
extern unsigned char inBuffer;			//bufor do którego są czytane pliki
extern uint16_t decBuffer;	//bufor do którego są dekodowane dane audio
extern uint16_t dmaBuffer;			//bufor do którego są kopiowane dane audio i z którego I2S gra muzykę
//extern enum DMABufferStateTypeDef DMABufferState;
extern volatile enum PlayerStateTypeDef PlayerState;
/* bufory do odczytu plików, dekodowani audio i odtwarzania dźwięku*/

/* zmienne zewnętrzne */

void WavProcess(FIL *audiFile)
{
	if (WavInit(audiFile)==0)
	{
		do
		{
			nRead = FillReadBuffer(&inBuffer, read_ptr, READ_BUFFER_SIZE, bytes_left, audiFile);
			bytes_left += nRead;

			if (WavDecode(&inBuffer, &decBuffer, sizeof(uint16_t) * DECODED_AUDIO_FRAME_SIZE_WAV))		//jeśli zdekodowało jakieś dane to czeka na synchronizację DMA i napełnia bufor DMA
			{
				nFrames++;
				if ((nFrames) % 100 == 0) {
					printf("WAVdec: frame %u\r\n", nFrames);//printf("WAVdec: frame %u, bitrate=%d\r\n", nFrames, mp3FrameInfo.bitrate);
				}

				playIfReady(&dmaBuffer, DMA_BUFFER_SIZE_WAV, (uint32_t) WAV_header.sampleRate);
				if (PlayerState != Player_error)
					wait4_I2S_DMA_SyncSignal();    // jeśli bufory sa pełne to czeka na synchronizację z przerwań buforów
				fill_DMAbuffer(&dmaBuffer, &decBuffer, DMA_BUFFER_SIZE_WAV);
			}
		}while (*&bytes_left&&(PlayerState != Player_error));// || (DMABufferState!=DMABuffer_Empty));		//odtwarza dopuki w buforze odczytu plikow sa jakieś dane
																		//i dopuki w buforze DMA coś jeszcze jest do odtworzena, ale lepiej bez tego, bo są wtedy trzaski w głośniku
	}
}

static bool WavDecode(uint8_t *inBuffer, uint16_t *decodeBuffer, uint32_t frameSize) //TODO: na konies ta funkcja ma być static
{
	offset = frameSize;

	//TODO: Dopisać sprawdzanie czy w buforze wejściowym są jeszcze jakieś nie obrobione dane. "Bytes left" != 0.

	if (bytes_left!=0)
	{
		if (WAV_header.numChannels==1)
		{
			offset = offset/2;		// jeśli plik audio jest w mono, to należy przekopiowac 2 razy mniej danych i je podwoić, aby dane szły na lewy i prawy kanał
		}
		memcpy(decodeBuffer,inBuffer, offset);
		read_ptr = inBuffer + offset;
		if (bytes_left >offset)	//sprawdza czy nie było końca czytania pliku i czy ilość danych w buforze pliku jest większa od ofsetu
			bytes_left = bytes_left - offset;
		else
			bytes_left = 0;
		if (WAV_header.numChannels==1)
		{
			convert_mono2stereo((uint16_t* )decodeBuffer, offset);
		}
		return TRUE;
	}
	else
	{
		//memset(decodeBuffer, NULL, DECODED_AUDIO_FRAME_SIZE_WAV);
		return FALSE;		//nie zdekodowano danyc, nie ma czego przesuwać do buforóow DMA
	}
}

static int	WavInit(FIL *wavFile) //, unsigned char *buffer, unsigned int buffer_size)
{
	UINT br;
	if (f_read(wavFile, &WAV_header, sizeof(WAV_header), &br)!= FR_OK)		// czytanie nagłówka pliku WAV
	{
		printf("WAVdec: header information not avaliable.\r\n");
		return 1;	// problem z dalszym odczytem pliku
	}

	if ((WAV_header.audioFormat != WAVE_FORMAT_PCM)															//sprawdza czy WAV ma wspierany format kodowania (PCM)
			||(!(WAV_header.numChannels==1 || WAV_header.numChannels==2))									//lub czy ilość kanałów jest odpowiednia (tylko mono i stereo)
			||(strncasecmp(&WAV_header.chunkID, "RIFF", 4)||strncasecmp(&WAV_header.format, "WAVE", 4)))	//lub czy nagłowki "RIFF" i "WAVE" są poprawne
	{
		if ((strncasecmp(&WAV_header.chunkID, "RIFF", 4)||strncasecmp(&WAV_header.format, "WAVE", 4)))
			printf("WAVdec: RIFF and/or WAHE header(s) not supported\r\n");
		if (WAV_header.audioFormat != WAVE_FORMAT_PCM)
			printf("WAVdec: format not supported (not PCM)\r\n");
		if (!(WAV_header.numChannels==1 || WAV_header.numChannels==2))
				printf("WAVdec: wrong channels quantity: %d\r\n", WAV_header.numChannels);
		return 2;	// Nie obsługiwany formt plików
	}
	printf_audio_information("WAV", WAV_header.sampleRate, WAV_header.bitsPerSample, WAV_header.numChannels);
	return 0;		// 0 zwraca jeśi z nagłówkiem wszystko jest w porządku
}
