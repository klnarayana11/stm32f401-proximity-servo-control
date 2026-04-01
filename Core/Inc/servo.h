/*
 * servo.h
 *
 *  Created on: Mar 28, 2026
 *      Author: KODI BALAJI
 */

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

void Servo_Init(TIM_HandleTypeDef *htim);
void Servo_SetAngle(uint8_t angle);
void Servo_SetZoneAngle(uint8_t zone);


#endif /* INC_SERVO_H_ */
