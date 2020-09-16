/*
 * play_mp3.h
 *
 *  Created on: 26 sie 2020
 *      Author: dbank
 */

#ifndef PLAYER_PLAY_MP3_H_
#define PLAYER_PLAY_MP3_H_

void Mp3Process(FIL *audiFile);
static uint32_t Mp3ReadId3V2Text(FIL* pInFile, uint32_t unDataLen, char* pszBuffer, uint32_t unBufferSize);
static uint32_t Mp3ReadId3V2Tag(FIL* pInFile, char* pszArtist, uint32_t unArtistSize, char* pszTitle, uint32_t unTitleSize);
static void printf_Mp3ReadId3V2Tag(FIL* pInFile);

#endif /* PLAYER_PLAY_MP3_H_ */
