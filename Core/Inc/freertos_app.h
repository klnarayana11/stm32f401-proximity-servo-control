/*
 * freertos_app.h
 *
 *  Created on: Mar 29, 2026
 *      Author: KODI BALAJI
 */

#ifndef INC_FREERTOS_APP_H_
#define INC_FREERTOS_APP_H_






/* ─────────────────────────────────────────
   INCLUDES
───────────────────────────────────────── */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "lcd.h"
#include "stm32f4xx_hal.h"

#include "ultrasonic.h"
#include "servo.h"

/* ─────────────────────────────────────────
   EXTERNAL TIMER HANDLES
   Defined in main.c by CubeMX
───────────────────────────────────────── */
extern TIM_HandleTypeDef htim3;   /* TIM3 — Servo PWM         */
extern TIM_HandleTypeDef htim2;   /* TIM2 — Ultrasonic IC     */

/* ─────────────────────────────────────────MX_FREERTOS_APP_Init();
   TASK CONFIGURATION
───────────────────────────────────────── */

/* Stack sizes (in words, 1 word = 4 bytes on Cortex-M4) */
#define SONAR_TASK_STACK_SIZE       256
#define SERVO_TASK_STACK_SIZE       256

/* Task priorities */
#define SONAR_TASK_PRIORITY         ( tskIDLE_PRIORITY + 2 )
#define SERVO_TASK_PRIORITY         ( tskIDLE_PRIORITY + 1 )

/* Task names */
#define SONAR_TASK_NAME             "Sonar"
#define SERVO_TASK_NAME             "Servo"

/* ─────────────────────────────────────────
   TIMING CONFIGURATION
───────────────────────────────────────── */
#define SONAR_TRIGGER_INTERVAL_MS   60U    /* HC-SR04 min retrigger gap  */
#define SONAR_ECHO_TIMEOUT_MS       50U    /* max wait for echo response */

/* ─────────────────────────────────────────
   QUEUE CONFIGURATION
───────────────────────────────────────── */
#define ZONE_QUEUE_LENGTH           1U     /* always holds latest zone only */

/* ─────────────────────────────────────────
   TASK HANDLE DECLARATIONS
   Defined in freertos_app.c
   extern so other files can notify/query
───────────────────────────────────────── */
extern TaskHandle_t   xSonarTaskHandle;
extern TaskHandle_t   xServoTaskHandle;
extern TaskHandle_t   xLcdTaskHandle;
//extern TaskHandle_t   xLcdTaskHandle;
/* ─────────────────────────────────────────
   QUEUE HANDLE DECLARATIONS
   Defined in freertos_app.c
   extern so ultrasonic driver ISR can access
───────────────────────────────────────── */
extern QueueHandle_t  xZoneQueue;
/* freertos_app.h */

/* ── LCD Queue Data — distance + zone sent to LCD task ── */
typedef struct {
    float          distanceCm;    // from ultrasonic driver
    DistanceZone_t zone;          // NEAR / MID / FAR
    uint8_t        angle;         // 0 / 90 / 180 degrees
} LCDData_t;

/* ── LCD Task Configuration ── */
#define LCD_TASK_STACK_SIZE      128        // 512 bytes
#define LCD_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define LCD_TASK_NAME            "LCD"
#define LCD_QUEUE_LENGTH         1          // always latest value

/* ── LCD Queue Handle ── */
extern QueueHandle_t  xLCDQueue;

/* ── LCD Task Declaration ── */
void vLCDTask(void *pvParameters);
/* ─────────────────────────────────────────
   PUBLIC FUNCTION DECLARATIONS
───────────────────────────────────────── */

/**
 * @brief  Creates all FreeRTOS tasks and queues.
 *         Call this before osKernelStart() in main.c
 */
void MX_FREERTOS_APP_Init(void);

/**
 * @brief  Sonar task — fires trigger, waits for ISR
 *         notification, reads zone, posts to queue.
 * @param  pvParameters  unused (pass NULL)
 */
void vSonarTask(void *pvParameters);
void vLcdTask(void *pvParameters);

/**
 * @brief  Servo task — waits for zone from queue,
 *         moves servo to mapped angle.
 * @param  pvParameters  unused (pass NULL)
 */
void vServoTask(void *pvParameters);

/**
 * @brief  HAL Input Capture callback override.
 *         Routes TIM2 events to ultrasonic driver.
 * @param  htim  pointer to timer handle from HAL ISR
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);


#endif /* INC_FREERTOS_APP_H_ */
