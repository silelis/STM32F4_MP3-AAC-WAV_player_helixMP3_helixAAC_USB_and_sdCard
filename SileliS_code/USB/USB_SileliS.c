/*
 * USB_SileliS.c
 *
 *  Created on: 10 lip 2020
 *      Author: dbank
 */

# include "USB_SileliS.h"




void USB_DEV_power_on(void)
{
	HAL_GPIO_WritePin(USB_power_on_port, USB_power_on_pin, GPIO_PIN_SET);
}

void USB_DEV_power_off(void)
{
	HAL_GPIO_WritePin(USB_power_on_port, USB_power_on_pin, GPIO_PIN_RESET);
}

static FRESULT USB_dev_mount_fs(void)
{
	FRESULT result = f_mount(&USBHFatFS /*myUSBHFatFS*/, (const TCHAR*) USBHPath, 0);
	if (result == FR_OK) {
		printf("FATFS: USB file system mounted.\r\n");
	} else {
		printf("FATFS: USB file system NOT mounted.\r\n");
	}
	return result;  // ponieważ nie zamontowano systemu plików
}

enum USB_dev_StateTypeDef USB_DEV_mount(ApplicationTypeDef Appli_state)
{
	switch(Appli_state)
	{
	case APPLICATION_IDLE:
		if (noticedUSBstate_last != USB_dev_IDLE) {
			noticedUSBstate_last = USB_dev_IDLE;
			printf("USB: idle\r\n");
		}
		break;
	case APPLICATION_START:
		if (noticedUSBstate_last != USB_dev_START) {
			//noticedUSBstate_last = noticedState_START;
			printf("USB: started\r\n");
				if (USB_dev_mount_fs() == FR_OK )
					noticedUSBstate_last = USB_dev_START;
				else
					noticedUSBstate_last = USB_dev_FS_mount_error;
				}
				break;
	case APPLICATION_READY:
		if (noticedUSBstate_last != USB_dev_READY) {
			if (noticedUSBstate_last == USB_dev_FS_mount_error){		//nie zamontowano FATFS
				if (USB_dev_mount_fs() != FR_OK ){
					noticedUSBstate_last = USB_dev_FS_mount_error;
				}
				else
				{
					noticedUSBstate_last = USB_dev_READY;
				}

			}
			else
			{
				noticedUSBstate_last = USB_dev_READY;
			}
		printf("USB: ready\r\n");
		}

//		if (noticedUSBstate_last == noticedState_READY)
//			printf("USB: ready\r\n");
		break;
	case APPLICATION_DISCONNECT:
		if (noticedUSBstate_last != USB_dev_DISCONNECT) {
			printf("USB: disonnected\r\n");
					FRESULT result = f_mount(0, (const TCHAR*) USBHPath/*""*/, 0);
					if (result == FR_OK) {
						//ff_free(&USBHFatFS);
						printf("FATFS: USB file system unmounted.\r\n");
						HAL_Delay(1);
					noticedUSBstate_last = USB_dev_DISCONNECT;
				} else {
					printf("FATFS: USB file system unmounted failed.\r\n");
				}
		}
		break;
	}
	return noticedUSBstate_last;
}
