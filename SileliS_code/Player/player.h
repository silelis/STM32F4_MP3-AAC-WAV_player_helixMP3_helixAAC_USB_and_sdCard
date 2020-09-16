/*
 * player.h
 *
 *  Created on: 29 lip 2020
 *      Author: dbank
 */

#ifndef PLAYER_PLAYER_H_
#define PLAYER_PLAYER_H_


//określenie wielkości buforów dla plikow AAC i MP4
#include "aacdec.h"
#define ARMULATE_MUL_FACT	1
#define MAX_FRAMES	-1
#ifdef AAC_ENABLE_SBR
	#define SBR_MUL		2
#else
	#define SBR_MUL		1
#endif

#define READBUF_SIZE_4_AAC				2048 /*ok*/	//3072	/*ok*/	//4096 /*ok*/	//(2 * AAC_MAINBUF_SIZE * AAC_MAX_NCHANS) /*ok*/
#define DECODED_AUDIO_FRAME_SIZE_AAC	AAC_MAX_NCHANS * AAC_MAX_NSAMPS * SBR_MUL
#define DMA_BUFFER_SIZE_AAC				2*DECODED_AUDIO_FRAME_SIZE_AAC

//określenie wielkości buforów dla plikow MP3
#include "mp3dec.h"
#define READBUF_SIZE_4_MP3				2048 /*ok*/	//3072 /*ok*/	//4096 /*ok*/
#define DECODED_AUDIO_FRAME_SIZE_MP3	MAX_NCHAN * MAX_NGRAN * MAX_NSAMP	//2304
#define DMA_BUFFER_SIZE_MP3				2*DECODED_AUDIO_FRAME_SIZE_MP3		//4608

#define DECODED_AUDIO_FRAME_SIZE_WAV	1000		//DECODED_AUDIO_FRAME_SIZE_MP3
#define DMA_BUFFER_SIZE_WAV				2000		//DMA_BUFFER_SIZE_MP3/2

//ostateczne okreslenie wielkości buforów dla plikow audio
#ifdef READBUF_SIZE_4_AAC
	#define READ_BUFFER_SIZE 				READBUF_SIZE_4_AAC
#else
	#define READ_BUFFER_SIZE 				READBUF_SIZE_4_MP3
#endif
#define DECODED_AUDIO_FRAME_SIZE		DECODED_AUDIO_FRAME_SIZE_MP3
#define DMA_BUFFER_SIZE 				DMA_BUFFER_SIZE_MP3

enum audioFileTypeDef{
	UNKNOWN,
	WAV,
	MP3,
	MP2,
	MP4,
	M4A,
	WMA,
	AAC,
	M4P,
	OGG
};


enum DMABufferStateTypeDef{
	DMABuffer_Full,
	DMABuffer_1stHalf_empty,
	DMABuffer_2ndHalf_empty,
	DMABuffer_Empty						//aka DMABuffer_1stHalf_empty | DMABuffer_2ndHalf_empty
};

enum PlayerStateTypeDef{
	Player_stopped,
	Player_playing,
	Player_paused,
	Player_error
};

void Player_reset(void);
static enum audioFileTypeDef get_filetype(char * filename);
void printf_audio_information(char* decoderName, unsigned long samprate, unsigned short bitsPerSample, unsigned short nChans);
void wait4_I2S_DMA_SyncSignal(void);
int FillReadBuffer(unsigned char *readBuf, unsigned char *readPtr, int bufSize, int bytesLeft, FILE *infile);
void fill_DMAbuffer(uint16_t* destinationBuffer, uint16_t* sourceBuffer, uint32_t destinationBufferSize);
void playIfReady(uint16_t* dmaBuffer2Play, uint16_t I2SdmaBufferSize, uint32_t sampleRate);
static void audioI2S_pllClockConfig(uint32_t audioFreq);
static void I2S3_freqUpdate(uint32_t AudioFreq);
//FRESULT play_directory (const char* path, unsigned char seek);
FRESULT play_directory (const char* path); //, unsigned char seek); //TODO: dorobić obsługę "seek"
void convert_mono2stereo(uint16_t* buffer, uint32_t buffer_size);
#endif /* PLAYER_PLAYER_H_ */
