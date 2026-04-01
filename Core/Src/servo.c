/*
 * servo.c
 *
 *  Created on: Mar 28, 2026
 *      Author: KODI Lakshmi Narayana
 */


/* servo.c */
#include "servo.h"
#include "freertos_app.h"
//#include "FreeRTOS.h"
/* ── Angle → CCR mapping for TIM1 at 100KHz, ARR=1999 ──
   1ms  = CCR 100  → 0°
   1.5ms= CCR 150  → 90°
   2ms  = CCR 200  → 180°               */
#define SERVO_MIN_CCR    1000
#define SERVO_MAX_CCR    2000

/* Zone angle definitions */
#define ANGLE_NEAR        0    // object very close  → 0°
#define ANGLE_MID        90    // object mid range   → 90°
#define ANGLE_FAR       180    // object far / clear → 180°

static TIM_HandleTypeDef *_hservo = NULL;

void Servo_Init(TIM_HandleTypeDef *htim)
{
    _hservo = htim;
    HAL_TIM_PWM_Start(_hservo, TIM_CHANNEL_1);
    Servo_SetAngle(90);   // center on startup
}

void Servo_SetAngle(uint8_t angle)
{
    if (angle > 180) angle = 180;
xx
    // Map 0–180° → CCR 100–200
    uint32_t ccr = SERVO_MIN_CCR +
                   ((uint32_t)angle * (SERVO_MAX_CCR - SERVO_MIN_CCR)) / 180;

    __HAL_TIM_SET_COMPARE(_hservo, TIM_CHANNEL_1, ccr);
    vTaskDelay(pdMS_TO_TICKS(500));
}

void Servo_SetZoneAngle(uint8_t zone)
{
    switch(zone)
    {
        case 0:  Servo_SetAngle(ANGLE_NEAR); break;  // NEAR → 0°
        case 1:  Servo_SetAngle(ANGLE_MID);  break;  // MID  → 90°
        case 2:  Servo_SetAngle(ANGLE_FAR);  break;  // FAR  → 180°
        default: Servo_SetAngle(90);         break;
    }
}
