/*
 * SileliS_printf.c
 *
 *  Created on: 29 lip 2020
 *      Author: dbank
 */

#include "SileliS_printf.h"
#include "stm32f4xx_hal.h"

static UART_HandleTypeDef *huart2redirect;

void printf_init(UART_HandleTypeDef *huart)
{
	huart2redirect = huart;
}

void __io_putchar(uint8_t ch) {
	HAL_UART_Transmit(huart2redirect, &ch, 1, 1);
}
