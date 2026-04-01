
#ifndef INC_LCD_H_
#define INC_LCD_H_
#include "stm32f4xx_hal.h"
#include <stdint.h>


#define TRIG_PIN_Pin GPIO_PIN_0
#define ECHO_PIN_GPIO_Port GPIOA
#define PWM_PIN_Pin GPIO_PIN_6
#define PWM_PIN_GPIO_Port GPIOA
#define LCD_D4_Pin GPIO_PIN_0
#define LCD_D4_GPIO_Port GPIOB
#define LCD_D5_Pin GPIO_PIN_1
#define LCD_D5_GPIO_Port GPIOB
#define LCD_D6_Pin GPIO_PIN_2
#define LCD_D6_GPIO_Port GPIOB
#define RED_LED_PIN_Pin GPIO_PIN_13
#define RED_LED_PIN_GPIO_Port GPIOB
#define GREEN_LED_PIN_Pin GPIO_PIN_14
#define GREEN_LED_PIN_GPIO_Port GPIOB
#define LCD_D7_Pin GPIO_PIN_3
#define LCD_D7_GPIO_Port GPIOB
#define LCD_RS_Pin GPIO_PIN_4
#define LCD_RS_GPIO_Port GPIOB
#define lCD_RW_Pin GPIO_PIN_5
#define lCD_RW_GPIO_Port GPIOB
#define LCD_EN_Pin GPIO_PIN_8
#define LCD_EN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define LM35_OUTPUT_Pin GPIO_PIN_0
#define LM35_OUTPUT_GPIO_Port GPIOC
#define POT_OUTPUT_Pin GPIO_PIN_1
#define POT_OUTPUT_GPIO_Port GPIOC
#define LCD_D4_Pin GPIO_PIN_0
#define LCD_D4_GPIO_Port GPIOB
#define LCD_D5_Pin GPIO_PIN_1
#define LCD_D5_GPIO_Port GPIOB
#define LCD_D6_Pin GPIO_PIN_2
#define LCD_D6_GPIO_Port GPIOB
#define RED_LED_Pin GPIO_PIN_13
#define RED_LED_GPIO_Port GPIOB
#define GREEN_LED_Pin GPIO_PIN_14
#define GREEN_LED_GPIO_Port GPIOB
#define LCD_D7_Pin GPIO_PIN_3
#define LCD_D7_GPIO_Port GPIOB
#define LCD_RS_Pin GPIO_PIN_4
#define LCD_RS_GPIO_Port GPIOB
#define LCD_RW_Pin GPIO_PIN_5
#define LCD_RW_GPIO_Port GPIOB
#define LCD_EN_Pin GPIO_PIN_8
#define LCD_EN_GPIO_Port GPIOB
void SystemClock_Config(void);

void LCD_Init(void);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t data);
void LCD_WriteString(char *str);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Clear(void);
void LCD_WriteHighNibble(uint8_t data);
 void LCD_WriteLowNibble(uint8_t data);
 void LCD_EnablePulse(void);
void LCD_WriteString(char *str);
void LCD_Clear(void);

 #define TRIG_PIN_GPIO_Port GPIOA
#define ECHO_PIN_Pin GPIO_PIN_1


#endif /* INC_LCD_H_ */
