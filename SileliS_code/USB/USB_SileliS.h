/*
 * USB_SileliS.h
 *
 *  Created on: 10 lip 2020
 *      Author: dbank
 */

#ifndef USB_SILELIS_H_
#define USB_SILELIS_H_

#include "usb_host.h"
#include "USB_SileliS_config.h"
#include "fatfs.h"

extern char USBHPath[4]; /* USBH logical drive path */
extern USBH_HandleTypeDef hUsbHostFS;
//FATFS myUSBHFatFS;
extern FATFS USBHFatFS; /* File system object for USBH logical drive */

enum USB_dev_StateTypeDef{
	USB_dev_READY,				// USB device is mounted and can be played (0)
	USB_dev_IDLE,
	USB_dev_START,
	USB_dev_DISCONNECT,
	USB_dev_FS_mount_error
};

static enum USB_dev_StateTypeDef noticedUSBstate_last;// = USB_dev_IDLE;

void USB_DEV_power_on(void);
void USB_DEV_power_off(void);
enum USB_dev_StateTypeDef USB_DEV_mount(ApplicationTypeDef Appli_state);  //sprawdza status podłączenia USB i zamontowania FAT na USB


#endif /* USB_SILELIS_H_ */
