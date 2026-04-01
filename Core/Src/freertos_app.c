/*
 * freertos_app.c
 *
 *  Created on: Mar 29, 2026
 *      Author: KODI BALAJI
 */
#include "stdio.h"
#include "lcd.h"
#include "main.h"
#include "freertos_app.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "ultrasonic.h"
#include "servo.h"
#define LED_PORT GPIOB
#define RED_LED_PIN  GPIO_PIN_13
#define GREEN_LED_PIN  GPIO_PIN_14
/* ── Handles ── */
TaskHandle_t      xSonarTaskHandle  = NULL;
TaskHandle_t      xServoTaskHandle  = NULL;
QueueHandle_t     xZoneQueue        = NULL;   // pass zone ISR → ServoTask
TaskHandle_t   xLcdTaskHandle   = NULL;
extern TIM_HandleTypeDef htim3;   // Servo PWM
extern TIM_HandleTypeDef htim2;   // Ultrasonic IC
extern UltrasonicData_t   sonarData ;
char buffer[20];

/* ────────────────────────────────MX_FREERTOS_APP_Init();─────────
   SONAR TASK
   - Fires trigger every 60ms
   - Waits for ISR notification
   - Reads zone, sends to ServoTask via queue
───────────────────────────────────────── */
void vSonarTask(void *pvParameters)
{
    DistanceZone_t zone;

    // Init ultrasonic, pass THIS task handle so ISR can notify us
    Ultrasonic_Init(&htim2, xSonarTaskHandle);

    for (;;)
    {
    	//HAL_GPIO_WritePin(LED_PORT, GREEN_LED_PIN,GPIO_PIN_SET);
        // 1. Fire trigger pulse
        Ultrasonic_Trigger();

        // 2. Block until ISR notifies (falling edge captured)
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50)); // 50ms timeout

        // 3. Read computed zone (ISR already classified it)
        zone = Ultrasonic_GetZone();

        // 4. Send zone to servo task via queue (non-blocking)
        xQueueOverwrite(xZoneQueue, &zone);

        // 5. Wait before next measurement (HC-SR04 needs ~60ms between triggers)
        vTaskDelay(pdMS_TO_TICKS(60));
    }
}

/* ─────────────────────────────────────────
   SERVO TASK
   - Waits for zone update from SonarTask
   - Moves servo to corresponding angle
───────────────────────────────────────── */
void vServoTask(void *pvParameters)
{   //HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
    HAL_Delay(500);
	Ultrasonic_Init(&htim2, xSonarTaskHandle);
    DistanceZone_t zone;
    DistanceZone_t lastZone = ZONE_FAR;   // track last zone to avoid redundant moves

    Servo_Init(&htim3);

    for (;;)
    {
        // Block until SonarTask sends a new zone
        if (xQueueReceive(xZoneQueue, &zone, portMAX_DELAY) == pdTRUE)
        {
            if (zone != lastZone)   // only move if zone changed
            {
                Servo_SetZoneAngle((uint8_t)zone);
                lastZone = zone;
            }
        }
    }
}

	/* ─────────────────────────────────────────
	   vLcdTask — FIXED & COMPLETE
	───────────────────────────────────────── */
	void vLcdTask(void *pvParameters)
	{
	    char line1[17];    // 16 chars + null terminator
	    char line2[17];

	    /* Init LCD — safe inside task, HAL_Delay works here */
	    LCD_Init();
	    vTaskDelay(pdMS_TO_TICKS(500));   // use vTaskDelay NOT HAL_Delay in RTOS

	    /* Startup message */
	    LCD_SetCursor(0, 0);
	    LCD_WriteString("  Sonar System  ");
	    LCD_SetCursor(1, 0);
	    LCD_WriteString("  Starting...   ");
	    vTaskDelay(pdMS_TO_TICKS(1500));
	    LCD_Clear();

	    for (;;)
	    {
	        /* ── Read latest values safely via getter functions ── */
	        float          dist  = Ultrasonic_GetDistance();
	        DistanceZone_t zone  = Ultrasonic_GetZone();

	        /* ── Compute angle from zone ── */
	        uint8_t angle;
	        switch(zone)
	        {
	            case ZONE_NEAR: angle =   0; break;
	            case ZONE_MID:  angle =  90; break;
	            case ZONE_FAR:  angle = 180; break;
	            default:        angle =  90; break;
	        }

	        /* ── Format Row 0: Distance ── */
	        int int_part = (int)dist;
	        int decimal  = (int)((dist - int_part) * 10);
	        snprintf(line1, sizeof(line1),
	                 "Dist:%3d.%1dcm   ",   // trailing spaces clear old chars
	                 int_part, decimal);

	        /* ── Format Row 1: Zone + Angle ── */
	        const char *zoneName;
	        switch(zone)
	        {
	            case ZONE_NEAR: zoneName = "NEAR"; break;
	            case ZONE_MID:  zoneName = "MID "; break;
	            case ZONE_FAR:  zoneName = "FAR "; break;
	            default:        zoneName = "----"; break;
	        }
	        snprintf(line2, sizeof(line2),
	                 "%4s Ang:%3d dg ",
	                 zoneName, angle);

	        /* ── Write to LCD ── */
	        LCD_SetCursor(0, 0);
	        LCD_WriteString(line1);

	        LCD_SetCursor(1, 0);
	        LCD_WriteString(line2);

	        /* ── Wait 500ms before next update ── */
	        /* LCD updates every 500ms — plenty fast for display */
	        vTaskDelay(pdMS_TO_TICKS(500));
	    }
	}
/* ─────────────────────────────────────────
   HAL Callback — routes to our driver
───────────────────────────────────────── */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    Ultrasonic_CaptureCallback(htim);   // delegate to ultrasonic driver
}

/* ─────────────────────────────────────────
   Create tasks & queue before scheduler
───────────────────────────────────────── */

  void MX_FREERTOS_APP_Init(void)
{
    /* ── Create queue first ── */
    xZoneQueue = xQueueCreate(1, sizeof(DistanceZone_t));

    /* ── Create tasks ── */
    xTaskCreate(vSonarTask, "Sonar", 256, NULL, 3, &xSonarTaskHandle);
    //                                            ↑ highest — time critical
    xTaskCreate(vServoTask, "Servo", 256, NULL, 2, &xServoTaskHandle);
    //                                            ↑ medium
    xTaskCreate(vLcdTask,   "Lcd",   256, NULL, 1, &xLcdTaskHandle);
    //                                            ↑ lowest — display not urgent
}

