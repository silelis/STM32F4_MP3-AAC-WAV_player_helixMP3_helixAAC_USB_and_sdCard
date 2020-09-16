/*
 * play_aac.c
 *
 *  Created on: 7 wrz 2020
 *      Author: dbank
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ff.h"
#include "player.h"
#include "aacdec.h"
#include "aaccommon.h"
#include "play_aac.h"


extern unsigned char	inBuffer;
extern uint16_t decBuffer;	//bufor do którego są dekodowane dane audio
extern uint16_t dmaBuffer;			//bufor do którego są kopiowane dane audio i z którego I2S gra muzykę

extern unsigned char	*read_ptr;
extern int err, outOfData, nFrames, bytes_left, nRead, eofReached;
extern uint32_t offset;
extern volatile enum PlayerStateTypeDef PlayerState;

void AacProcess(FIL *audiFile, int RAW)
{
	  int nResult = 0;
	  Player_reset();
	  char decTYPE [4];

	  //AACDecInfo 	aacDecInfo;	//TODO: obsługa mp4 nie działa
	  AACFrameInfo	aacFrameInfo;
	  HAACDecoder hAACDecoder = AACInitDecoder();




		  if (RAW==AACdec)
			  memcpy(&decTYPE,"Aac\0",4);
		  else
		  {
			  memcpy(&decTYPE,"Mp4\0",4);
			  //TODO: obsługa mp4 nie działa
			 /* aacDecInfo.format = AAC_FF_Unknown; //AAC_FF_RAW;
			  aacDecInfo.profile = AAC_PROFILE_LC;
			  AACSetRawBlockParams(&hAACDecoder, 0, &aacFrameInfo);*/
			  //TODO: obsługa mp4 nie działa
		  }

	  if(hAACDecoder == NULL)
	  {


		  printf("%sDec: Failed to initialize aac decoder engine\r\n",decTYPE);
		  //break;
	    //return -2;
	  }
	  else
	  {
		  printf("%sDec: Start decoding\r\n", decTYPE);
		  do
		  {
			  if (bytes_left < AAC_MAX_NCHANS * AAC_MAINBUF_SIZE && !eofReached){	//if (bytes_left < (2 * AAC_MAINBUF_SIZE) && (!eofReached)) {
					nRead = FillReadBuffer(&inBuffer, read_ptr, READ_BUFFER_SIZE, bytes_left, audiFile);
					bytes_left += nRead;
					read_ptr = &inBuffer;
					if (nRead == 0) {
						eofReached= 1;
					}
				}


//TODO: obsługa mp4 nie działa
/*				if (!RAW) {
					offset = AACFindSyncWord(read_ptr, bytes_left);
					if (offset < 0) {
						outOfData = TRUE;
						break;
					}
					read_ptr += offset;
					bytes_left -= offset;
					}*/
//TODO: obsługa mp4 nie działa




			  err = AACDecode(hAACDecoder, &read_ptr, (int*) &bytes_left, (short*) &decBuffer);
			  switch (err) {

			  case ERR_AAC_NONE :{
				  AACGetLastFrameInfo(hAACDecoder, &aacFrameInfo);
				  if (nFrames== 0){
					  printf_audio_information(decTYPE, aacFrameInfo.sampRateOut, aacFrameInfo.bitsPerSample, aacFrameInfo.nChans);
						if ((aacFrameInfo.sampRateOut > 48000)
								|| (aacFrameInfo.bitsPerSample != 16)
								|| ((aacFrameInfo.nChans != 2)&&(aacFrameInfo.nChans != 1)    )){	//|| (mp3FrameInfo.nChans != 2)) {
							printf("%sDec: incompatible %s file.\r\n", decTYPE, decTYPE);
							nResult = -5;
							break;
						}
				  }

				  if (aacFrameInfo.nChans == 1)
						convert_mono2stereo(&decBuffer, DECODED_AUDIO_FRAME_SIZE_AAC);	//TODO: SPRAWDZIĆ DZIALANIE TEGO KODU

					if ((nFrames) % 100 == 0) {
						printf("%sDec: frame %u, bitrate=%d\r\n", decTYPE, nFrames,
								aacFrameInfo.bitRate);
					}
					nFrames++;

					playIfReady(&dmaBuffer, DMA_BUFFER_SIZE_AAC, (uint32_t) aacFrameInfo.sampRateOut);
					if (PlayerState != Player_error)
						wait4_I2S_DMA_SyncSignal();    // jeśli bufory sa pełne to czeka na synchronizację z przerwań buforów
					fill_DMAbuffer(&dmaBuffer, &decBuffer, DMA_BUFFER_SIZE_AAC);
					break;


				  break;
			  	  }
//				case ERR_AAC_INVALID_ADTS_HEADER:{
//					break;
//					}
				case ERR_AAC_INDATA_UNDERFLOW /*ERR_MP3_INDATA_UNDERFLOW*/: {
					printf("%sDec: Decoding error ERR_AAC_INDATA_UNDERFLOW\r\n", decTYPE);
					outOfData = TRUE;
					break;
					}
				default: {
					printf("%sDec: Decoding error %d\r\n", decTYPE, err);
					outOfData = TRUE;
					break;
					}
			  }
		  }while((!outOfData) && (nResult == 0)&&(PlayerState != Player_error)); //while((!outOfData) && (nResult == 0));

		  printf("%sDec: Finished decoding\r\n", decTYPE);
	}
	  AACFreeDecoder(hAACDecoder);

}
