/*
 * hal_enc28j60.h
 *
 *  Created on: Sep 6, 2024
 *      Author: Konstantin
 */

#ifndef HAL_ENC28J60_CA1D0A3973624C09_H_
#define HAL_ENC28J60_CA1D0A3973624C09_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "spi.h"

#define ENC28J60_RESET_HI()	 HAL_GPIO_WritePin(ENC_RESET_GPIO_Port, ENC_RESET_Pin, GPIO_PIN_SET);
#define ENC28J60_RESET_LOW()	 HAL_GPIO_WritePin(ENC_RESET_GPIO_Port, ENC_RESET_Pin, GPIO_PIN_RESET);

#define ENC28J60_CS_HI()	 	 HAL_GPIO_WritePin(ENC_CS_GPIO_Port, ENC_CS_Pin, GPIO_PIN_SET);
#define ENC28J60_CS_LOW()	  	 HAL_GPIO_WritePin(ENC_CS_GPIO_Port, ENC_CS_Pin, GPIO_PIN_RESET);

void enc28j60_reset(void);
void enc28j60_error(void);
void enc28j60_cs_hi(void);
void enc28j60_cs_low(void);
int enc28j60_write_data (uint8_t *data, uint32_t len);
int enc28j60_read_data (uint8_t *data, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif /* HAL_ENC28J60_CA1D0A3973624C09_H_ */
