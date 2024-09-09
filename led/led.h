/*
 * led.h
 *
 *  Created on: Sep 6, 2024
 *      Author: Konstantin
 */

#ifndef LED_B05F5363E2664480_H_
#define LED_B05F5363E2664480_H_

#include "main.h"

#define LED0_ON()	 HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
#define LED0_OFF()	 HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);

#define LED1_ON()	 HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
#define LED1_OFF()	 HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

void led0_on (void);
void led0_off (void);
void led1_on (void);
void led1_off (void);

#endif /* LED_H_ */
