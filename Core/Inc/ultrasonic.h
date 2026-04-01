/*
 * ultrasonic.h
 *
 *  Created on: Mar 28, 2026
 *      Author: KODI BALAJI
 */
#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

/* -----------------------------
   Zone thresholds (in cm)
------------------------------ */
#define ZONE_NEAR_MAX_CM   10.0f
#define ZONE_MID_MAX_CM    25.0f

/* -----------------------------
   Distance zones
------------------------------ */
typedef enum
{
    ZONE_NEAR = 0,
    ZONE_MID,
    ZONE_FAR
} DistanceZone_t;

/* -----------------------------
   Ultrasonic data structure
------------------------------ */
typedef struct
{
    uint32_t risingCapture;
    uint32_t fallingCapture;
    float distanceCm;
    DistanceZone_t zone;
    uint8_t edgeState;   // 0 = waiting for rising, 1 = waiting for falling
} UltrasonicData_t;

/* -----------------------------
   Function Prototypes
------------------------------ */
void Ultrasonic_Init(TIM_HandleTypeDef *htim, TaskHandle_t taskHandle);
void Ultrasonic_Trigger(void);
void Ultrasonic_CaptureCallback(TIM_HandleTypeDef *htim);

float Ultrasonic_GetDistance(void);
DistanceZone_t Ultrasonic_GetZone(void);

#endif /* INC_ULTRASONIC_H_ */
