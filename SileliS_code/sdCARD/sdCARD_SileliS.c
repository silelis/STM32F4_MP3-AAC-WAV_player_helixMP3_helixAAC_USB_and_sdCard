/*
 * sdCARD_SileliS.c
 *
 *  Created on: Sep 14, 2020
 *      Author: dbank
 */


#include "sdCARD_SileliS.h"

ApplicationTypeDef sdCardAppli_state = APPLICATION_IDLE;



sdCARD_HOST_Process(void)	//TODO: zmodyfikować tę funkcję jeśli będzie sdCARD pin detect
{
	if((BSP_SD_Init()==MSD_OK) && (sdCardAppli_state == APPLICATION_IDLE))
		sdCardAppli_state = APPLICATION_START;
	if((BSP_SD_Init()==MSD_OK) && (sdCardAppli_state == APPLICATION_DISCONNECT))
		sdCardAppli_state = APPLICATION_START;
	if ((BSP_SD_Init()==MSD_OK) && (noticedSdCARDstate_last == sdCARD_dev_START))
		sdCardAppli_state = APPLICATION_READY;
	if ((BSP_SD_Init()!=MSD_OK)&&(sdCardAppli_state != APPLICATION_IDLE))
		sdCardAppli_state = APPLICATION_DISCONNECT;
}

static FRESULT sdCARD_dev_mount_fs(void)
{
	FRESULT result = f_mount(&SDFatFS /*myUSBHFatFS*/, (const TCHAR*) SDPath, 0);
	if (result == FR_OK) {
		printf("FATFS: sdCARD file system mounted.\r\n");
	} else {
		printf("FATFS: sdCARD file system NOT mounted.\r\n");
	}
	return result;  // ponieważ nie zamontowano systemu plików
}

enum sdCARD_dev_StateTypeDef sdCARD_DEV_mount (ApplicationTypeDef Appli_state)
{
	switch(Appli_state)
	{

	case APPLICATION_IDLE:
		if (noticedSdCARDstate_last != sdCARD_dev_IDLE) {
			noticedSdCARDstate_last = sdCARD_dev_IDLE;
			printf("sdCARD: idle\r\n");
		}
		break;

	case APPLICATION_START:
		if (noticedSdCARDstate_last != sdCARD_dev_START) {
			printf("sdCARD: started\r\n");
				if (sdCARD_dev_mount_fs() == FR_OK )				//todo: zrobić tą funkcję
					noticedSdCARDstate_last = sdCARD_dev_START;
				else
					noticedSdCARDstate_last = sdCARD_dev_FS_mount_error;
				}
				break;
	case APPLICATION_READY:
		if (noticedSdCARDstate_last != sdCARD_dev_READY) {
			if (noticedSdCARDstate_last == sdCARD_dev_FS_mount_error){		//nie zamontowano FATFS
				if (sdCARD_dev_mount_fs() != FR_OK ){				//todo: zrobić tą funkcję
					noticedSdCARDstate_last = sdCARD_dev_FS_mount_error;
				}
				else
				{
					noticedSdCARDstate_last = sdCARD_dev_READY;
				}

			}
			else
			{
				noticedSdCARDstate_last = sdCARD_dev_READY;
			}
		printf("sdCARD: ready\r\n");
		}
		break;

	case APPLICATION_DISCONNECT:
		if (noticedSdCARDstate_last != sdCARD_dev_DISCONNECT) {
			printf("sdCARD: disonnected\r\n");
					FRESULT result = f_mount(0, (const TCHAR*) SDPath, 0);
					if (result == FR_OK) {
						//ff_free(&USBHFatFS);
						printf("FATFS: sdCARD file system unmounted.\r\n");
						HAL_Delay(1);
						noticedSdCARDstate_last= sdCARD_dev_DISCONNECT;
				} else {
					printf("FATFS: sdCARD file system unmounted failed.\r\n");
				}
		}
		break;
	}
	return noticedSdCARDstate_last;




}



//sdCARD_dev_mount_fs()
