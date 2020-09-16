/*
 * play_mp3.c
 *
 *  Created on: 26 sie 2020
 *      Author: dbank
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ff.h"
#include "player.h"
#include "mp3dec.h"
#include "play_mp3.h"

extern unsigned char	inBuffer;
extern uint16_t decBuffer;	//bufor do którego są dekodowane dane audio
extern uint16_t dmaBuffer;			//bufor do którego są kopiowane dane audio i z którego I2S gra muzykę

extern unsigned char	*read_ptr;
extern int err, outOfData, nFrames, bytes_left, nRead, eofReached;
extern uint32_t offset;
extern volatile enum PlayerStateTypeDef PlayerState;


//int Mp3Decode(const char* pszFile)
//{
//  int nResult = 0;				//TODO
//  Player_reset();
//
//  FIL fIn;						//TODO
//  MP3FrameInfo mp3FrameInfo;
//
//  FRESULT errFS = f_open(&fIn, pszFile, FA_READ);		//TODO
//  if(errFS != FR_OK)
//  {
//    printf("Mp3Dec: Failed to open file \"%s\" for reading, err=%d\r\n", pszFile, errFS);
//    return -1;
//  }
//
//
//  HMP3Decoder hMP3Decoder = MP3InitDecoder();
//  if(hMP3Decoder == NULL)
//  {
//	  printf("Mp3Dec: Failed to initialize mp3 decoder engine\r\n");
//    return -2;
//  }
//
//  printf("Mp3Dec: Start decoding \"%s\"\r\n", pszFile);
//
//  {
//	  char szArtist[80];			//TODO
//	  char szTitle[80];				//TODO
//	  Mp3ReadId3V2Tag(&fIn, szArtist, sizeof(szArtist), szTitle, sizeof(szTitle));
//	  if(szArtist[0] != 0 || szTitle[0] != 0)
//	  {
//		  printf("Mp3Dec: Now playing (ID3v2): %s - %s\r\n", szArtist, szTitle);
//	  }
//  }
//
//  do
//  {
//		if (bytes_left < (2 * MAINBUF_SIZE) && (!eofReached /*bEof*/)) {
//
//			nRead = FillReadBuffer(&inBuffer, read_ptr, READ_BUFFER_SIZE, bytes_left, &fIn);
//			bytes_left += nRead;
//			read_ptr = &inBuffer;
//			if (nRead == 0) {
//				eofReached /*bEof*/ = 1;
//			}
//		}
//
//		// find start of next MP3 frame - assume EOF if no sync found
//		offset = MP3FindSyncWord(read_ptr, bytes_left);
//		if (offset < 0) {
//			outOfData = TRUE;
//			break;
//		}
//		read_ptr += offset;
//		bytes_left -= offset;
//
//		// decode one MP3 frame - if offset < 0 then bytesLeft was less than a full frame
//		err = MP3Decode(hMP3Decoder, &read_ptr, (int*) &bytes_left, (short*) &decBuffer, 0);
//
//		switch (err) {
//		case ERR_MP3_NONE: {
//			MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
//			if (nFrames== 0) {
//				printf("Mp3Dec: %d Hz %d Bit %d Channels\r\n",
//						mp3FrameInfo.samprate, mp3FrameInfo.bitsPerSample,
//						mp3FrameInfo.nChans);
//				if ((mp3FrameInfo.samprate > 48000)
//						|| (mp3FrameInfo.bitsPerSample != 16)
//						|| (mp3FrameInfo.nChans != 2)) {
//					printf("Mp3Dec: incompatible MP3 file.\r\n");
//					nResult = -5;
//					break;
//				}
//			}
//			if ((nFrames) % 100 == 0) {
//				printf("Mp3Dec: frame %u, bitrate=%d\r\n", nFrames,
//						mp3FrameInfo.bitrate);
//			}
//			nFrames++;
//
//			playIfReady(&dmaBuffer, DMA_BUFFER_SIZE_MP3, (uint32_t) mp3FrameInfo.samprate);
//			if (PlayerState != Player_error)
//				wait4_I2S_DMA_SyncSignal();    // jeśli bufory sa pełne to czeka na synchronizację z przerwań buforów
//			fill_DMAbuffer(&dmaBuffer, &decBuffer, DMA_BUFFER_SIZE_MP3);
//			break;
//		}
//		case ERR_MP3_MAINDATA_UNDERFLOW: {
//			// do nothing - next call to decode will provide more mainData
//			break;
//		}
//		case ERR_MP3_FREE_BITRATE_SYNC: {
//			break;
//		}
//		case ERR_MP3_INDATA_UNDERFLOW: {
//			printf("Mp3Decode: Decoding error ERR_MP3_INDATA_UNDERFLOW\r\n");
//			outOfData = TRUE;
//			break;
//		}
//		default: {
//			printf("Mp3Decode: Decoding error %d\r\n", err);
//			outOfData = TRUE;
//			break;
//		}
//		}
//  }
//  while((!outOfData) && (nResult == 0)&&(PlayerState != Player_error)); //while((!outOfData) && (nResult == 0));
//
//  printf("Mp3Decode: Finished decoding\r\n");
//
//  MP3FreeDecoder(hMP3Decoder);
//
//  f_close(&fIn);
//
//}


void Mp3Process(FIL *audiFile)
{
	  int nResult = 0;
	  Player_reset();

	  MP3FrameInfo mp3FrameInfo;

	  HMP3Decoder hMP3Decoder = MP3InitDecoder();
	  if(hMP3Decoder == NULL)
	  {
		  printf("Mp3Dec: Failed to initialize mp3 decoder engine\r\n");
		  //break;
		  //return -2;
	  }
	  else
	  {

		  printf("Mp3Dec: Start decoding\r\n");//printf("Mp3Dec: Start decoding \"%s\"\r\n", audiFile);

		  printf_Mp3ReadId3V2Tag(audiFile);

		  do
		  {
				if (bytes_left < (2 * MAINBUF_SIZE) && (!eofReached)) {
					nRead = FillReadBuffer(&inBuffer, read_ptr, READ_BUFFER_SIZE, bytes_left, audiFile);
					bytes_left += nRead;
					read_ptr = &inBuffer;
					if (nRead == 0) {
						eofReached= 1;
					}
				}

				// find start of next MP3 frame - assume EOF if no sync found
				offset = MP3FindSyncWord(read_ptr, bytes_left);
				if (offset < 0) {
					outOfData = TRUE;
					break;
				}
				read_ptr += offset;
				bytes_left -= offset;

				// decode one MP3 frame - if offset < 0 then bytesLeft was less than a full frame


				//memset(decBuffer,NULL,sizeof(decBuffer));		//TODO: usunąć to w ostatecznej wersji

				err = MP3Decode(hMP3Decoder, &read_ptr, (int*) &bytes_left, (short*) &decBuffer, 0);

				switch (err) {
				case ERR_MP3_NONE: {
					MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
					if (nFrames== 0) {
						printf_audio_information("Mp3", mp3FrameInfo.samprate, mp3FrameInfo.bitsPerSample, mp3FrameInfo.nChans);

						if ((mp3FrameInfo.samprate > 48000)
								|| (mp3FrameInfo.bitsPerSample != 16)
								|| ((mp3FrameInfo.nChans != 2)&&(mp3FrameInfo.nChans != 1)    )){	//|| (mp3FrameInfo.nChans != 2)) {
							printf("Mp3Dec: incompatible MP3 file.\r\n");
							nResult = -5;
							break;
						}
					}

	//				//konwersja z mono na stereo
					if (mp3FrameInfo.nChans == 1)
						convert_mono2stereo(&decBuffer, DECODED_AUDIO_FRAME_SIZE_MP3);

					if ((nFrames) % 100 == 0) {
						printf("Mp3Dec: frame %u, bitrate=%d\r\n", nFrames,
								mp3FrameInfo.bitrate);
					}
					nFrames++;

					playIfReady(&dmaBuffer, DMA_BUFFER_SIZE_MP3, (uint32_t) mp3FrameInfo.samprate);
					if (PlayerState != Player_error)
						wait4_I2S_DMA_SyncSignal();    // jeśli bufory sa pełne to czeka na synchronizację z przerwań buforów
					fill_DMAbuffer(&dmaBuffer, &decBuffer, DMA_BUFFER_SIZE_MP3);
					break;
				}
				case ERR_MP3_MAINDATA_UNDERFLOW: {
					// do nothing - next call to decode will provide more mainData
					break;
				}
				case ERR_MP3_FREE_BITRATE_SYNC: {
					break;
				}
				case ERR_MP3_INDATA_UNDERFLOW: {
					printf("Mp3Dec: Decoding error ERR_MP3_INDATA_UNDERFLOW\r\n");
					outOfData = TRUE;
					break;
				}
				default: {
					printf("Mp3Dec: Decoding error %d\r\n", err);
					outOfData = TRUE;
					break;
				}
				}
		  }
		  while((!outOfData) && (nResult == 0)&&(PlayerState != Player_error)); //while((!outOfData) && (nResult == 0));

		  printf("Mp3Dec: Finished decoding\r\n");
	  }
	  MP3FreeDecoder(hMP3Decoder);
}

static uint32_t Mp3ReadId3V2Text(FIL* pInFile, uint32_t unDataLen, char* pszBuffer, uint32_t unBufferSize)
{
  UINT unRead = 0;
  BYTE byEncoding = 0;
  if((f_read(pInFile, &byEncoding, 1, &unRead) == FR_OK) && (unRead == 1))
  {
    unDataLen--;
    if(unDataLen <= (unBufferSize - 1))
    {
      if((f_read(pInFile, pszBuffer, unDataLen, &unRead) == FR_OK) ||
          (unRead == unDataLen))
      {
        if(byEncoding == 0)
        {
          // ISO-8859-1 multibyte
          // just add a terminating zero
          pszBuffer[unDataLen] = 0;
        }
        else if(byEncoding == 1)
        {
          // UTF16LE unicode
          uint32_t r = 0;
          uint32_t w = 0;
          if((unDataLen > 2) && (pszBuffer[0] == 0xFF) && (pszBuffer[1] == 0xFE))
          {
            // ignore BOM, assume LE
            r = 2;
          }
          for(; r < unDataLen; r += 2, w += 1)
          {
            // should be acceptable for 7 bit ascii
            pszBuffer[w] = pszBuffer[r];
          }
          pszBuffer[w] = 0;
        }
      }
      else
      {
        return 1;
      }
    }
    else
    {
      // we won't read a partial text
      if(f_lseek(pInFile, f_tell(pInFile) + unDataLen) != FR_OK)
      {
        return 1;
      }
    }
  }
  else
  {
    return 1;
  }
  return 0;
}

static uint32_t Mp3ReadId3V2Tag(FIL* pInFile, char* pszArtist, uint32_t unArtistSize, char* pszTitle, uint32_t unTitleSize)
{
  //pszArtist[0] = 0;
  //pszTitle[0] = 0;

  BYTE id3hd[10];
  UINT unRead = 0;
  if((f_read(pInFile, id3hd, 10, &unRead) != FR_OK) || (unRead != 10))
  {
    return 1;
  }
  else
  {
    uint32_t unSkip = 0;
    if((unRead == 10) &&
        (id3hd[0] == 'I') &&
        (id3hd[1] == 'D') &&
        (id3hd[2] == '3'))
    {
      unSkip += 10;
      unSkip = ((id3hd[6] & 0x7f) << 21) | ((id3hd[7] & 0x7f) << 14) | ((id3hd[8] & 0x7f) << 7) | (id3hd[9] & 0x7f);

      // try to get some information from the tag
      // skip the extended header, if present
      uint8_t unVersion = id3hd[3];
      if(id3hd[5] & 0x40)
      {
        BYTE exhd[4];
        f_read(pInFile, exhd, 4, &unRead);
        size_t unExHdrSkip = ((exhd[0] & 0x7f) << 21) | ((exhd[1] & 0x7f) << 14) | ((exhd[2] & 0x7f) << 7) | (exhd[3] & 0x7f);
        unExHdrSkip -= 4;
        if(f_lseek(pInFile, f_tell(pInFile) + unExHdrSkip) != FR_OK)
        {
          return 1;
        }
      }
      uint32_t nFramesToRead = 2;
      while(nFramesToRead > 0)
      {
        char frhd[10];
        if((f_read(pInFile, frhd, 10, &unRead) != FR_OK) || (unRead != 10))
        {
          return 1;
        }
        if((frhd[0] == 0) || (strncmp(frhd, "3DI", 3) == 0))
        {
          break;
        }
        char szFrameId[5] = {0, 0, 0, 0, 0};
        memcpy(szFrameId, frhd, 4);
        uint32_t unFrameSize = 0;
        uint32_t i = 0;
        for(; i < 4; i++)
        {
          if(unVersion == 3)
          {
            // ID3v2.3
            unFrameSize <<= 8;
            unFrameSize += frhd[i + 4];
          }
          if(unVersion == 4)
          {
            // ID3v2.4
            unFrameSize <<= 7;
            unFrameSize += frhd[i + 4] & 0x7F;
          }
        }

        if(strcmp(szFrameId, "TPE1") == 0)
        {
          // artist
          if(Mp3ReadId3V2Text(pInFile, unFrameSize, pszArtist, unArtistSize) != 0)
          {
            break;
          }
          nFramesToRead--;
        }
        else if(strcmp(szFrameId, "TIT2") == 0)
        {
          // title
          if(Mp3ReadId3V2Text(pInFile, unFrameSize, pszTitle, unTitleSize) != 0)
          {
            break;
          }
          nFramesToRead--;
        }
        else
        {
          if(f_lseek(pInFile, f_tell(pInFile) + unFrameSize) != FR_OK)
          {
            return 1;
          }
        }
      }
      printf ("Mp3Dec: Skipping %u bytes of ID3v2 tag\r\n", unSkip + 1);
    }
    if(f_lseek(pInFile, unSkip) != FR_OK)
    {
      return 1;
    }
  }

  return 0;
}

static void printf_Mp3ReadId3V2Tag(FIL* pInFile)
{
	char* pszArtist = malloc(sizeof(char)*80);
	char* pszTitle = malloc(sizeof(char)*80);
	memset(pszArtist, 0, sizeof(char)*80);
	memset(pszTitle, 0, sizeof(char)*80);
	Mp3ReadId3V2Tag(pInFile, pszArtist, sizeof(char)*80, pszTitle, sizeof(char)*80);
	if(* pszArtist != 0 || * pszTitle != 0)
	{
		  printf("Mp3Dec: Now playing (ID3v2): %s - %s\r\n", pszArtist, pszTitle);
	}
	free(pszArtist);
	free(pszTitle);
	//czytanie Tagów MP3
}
