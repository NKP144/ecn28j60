/*
 * hal_enc28j60.c
 *
 *  Created on: Sep 6, 2024
 *      Author: Konstantin
 */

#include "hal_enc28j60.h"

void enc28j60_reset(void)
{
	ENC28J60_RESET_LOW();
	HAL_Delay(200);
	ENC28J60_RESET_HI();
	HAL_Delay(200);
}

void enc28j60_error(void)
{
	led0_on();
}

void enc28j60_cs_hi(void)
{
	ENC28J60_CS_HI();
}

void enc28j60_cs_low(void)
{
	ENC28J60_CS_LOW();
}

int enc28j60_write_data (uint8_t *data, uint32_t len)
{
	if(HAL_SPI_Transmit(&hspi2, data, len, 0x1000) != HAL_OK)
	{
		enc28j60_error();
		return -1;
	}

	return 0;
}

int enc28j60_read_data (uint8_t *data, uint32_t len)
{
	if(HAL_SPI_Receive(&hspi2, data, len, 0x1000) != HAL_OK)
	{
		enc28j60_error();
		return -1;
	}

	return 0;
}
