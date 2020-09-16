/*
 * sdCARD_SileliS.h
 *
 *  Created on: Sep 14, 2020
 *      Author: dbank
 */

#ifndef SDCARD_SDCARD_SILELIS_H_
#define SDCARD_SDCARD_SILELIS_H_


#include "sdCARD_SileliS_config.h"
#include "usb_host.h"		//stąd bierze ApplicationTypeDef
#include "fatfs.h"

extern char SDPath[4]; /* SD logical drive path */
extern FATFS SDFatFS; /* File system object for SD logical drive */

enum sdCARD_dev_StateTypeDef{
	sdCARD_dev_READY,				// USB device is mounted and can be played (0)
	sdCARD_dev_IDLE,
	sdCARD_dev_START,
	sdCARD_dev_DISCONNECT,
	sdCARD_dev_FS_mount_error
} ;

static enum sdCARD_dev_StateTypeDef noticedSdCARDstate_last;




extern char USBHPath[4]; /* USBH logical drive path */
extern USBH_HandleTypeDef hUsbHostFS;
//FATFS myUSBHFatFS;
extern FATFS USBHFatFS; /* File system object for USBH logical drive */



#endif /* SDCARD_SDCARD_SILELIS_H_ */
