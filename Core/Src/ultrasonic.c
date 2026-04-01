/*
 * ultrasonic.c
 *
 *  Created on: Mar 28, 2026
 *      Author: KODI lakshmi narayana
 */
#include "ultrasonic.h"
#include "cmsis_os.h"

/* ── Private state ── */
 UltrasonicData_t   sonarData   = {0};
static TIM_HandleTypeDef *_htim       = NULL;
static TaskHandle_t       _taskHandle = NULL;

/* -----------------------------------------
   Init: store references, enable IC + DWT
------------------------------------------ */
void Ultrasonic_Init(TIM_HandleTypeDef *htim, TaskHandle_t taskHandle)
{
    _htim       = htim;
    _taskHandle = taskHandle;

    /* Enable DWT cycle counter for 10us trigger pulse */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* Start input capture interrupt on CH2 */
    HAL_TIM_IC_Start_IT(_htim, TIM_CHANNEL_2);
}

/* -----------------------------------------
   Trigger: send 10us pulse on TRIG pin
------------------------------------------ */
void Ultrasonic_Trigger(void)
{
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0, GPIO_PIN_SET);

    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < (16 * 10));   // 84 MHz → 10us

    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0, GPIO_PIN_RESET);
}

/* -----------------------------------------
   Callback: called by HAL from TIM ISR
------------------------------------------ */
void Ultrasonic_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM2) return;
    if (htim->Channel  != HAL_TIM_ACTIVE_CHANNEL_2) return;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (sonarData.edgeState == 0)
    {
        /* Rising edge */
        sonarData.risingCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        sonarData.edgeState = 1;

        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2,
                                      TIM_INPUTCHANNELPOLARITY_FALLING);
    }
    else
    {
        /* Falling edge */
        sonarData.fallingCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        sonarData.edgeState = 0;

        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2,
                                      TIM_INPUTCHANNELPOLARITY_RISING);

        uint32_t pulseWidth;
        if (sonarData.fallingCapture >= sonarData.risingCapture)
            pulseWidth = sonarData.fallingCapture - sonarData.risingCapture;
        else
            pulseWidth = (0xFFFF - sonarData.risingCapture) + sonarData.fallingCapture;

        /* Distance in cm (1 tick = 1us) */
        sonarData.distanceCm = (float)pulseWidth / 58.0f;

        /* Zone classification */
        if (sonarData.distanceCm <= ZONE_NEAR_MAX_CM)
            sonarData.zone = ZONE_NEAR;
        else if (sonarData.distanceCm <= ZONE_MID_MAX_CM)
            sonarData.zone = ZONE_MID;
        else
            sonarData.zone = ZONE_FAR;

        /* Notify Sonar Task */
        vTaskNotifyGiveFromISR(_taskHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* -----------------------------------------
   Public getters
------------------------------------------ */
float Ultrasonic_GetDistance(void)
{
    return sonarData.distanceCm;
}

DistanceZone_t Ultrasonic_GetZone(void)
{
    return sonarData.zone;
}

