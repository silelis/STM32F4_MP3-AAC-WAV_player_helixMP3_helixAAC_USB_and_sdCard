/*
 * play_aac.h
 *
 *  Created on: 7 wrz 2020
 *      Author: dbank
 */

#ifndef PLAYER_PLAY_AAC_H_
#define PLAYER_PLAY_AAC_H_

#define AACdec	0
#define MP4dec	1

void AacProcess(FIL *audiFile, int raw);

#endif /* PLAYER_PLAY_AAC_H_ */
