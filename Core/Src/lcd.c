/*
 * lcd.c
 *
 *  Created on: Mar 29, 2026
 *      Author: KODI
 */
#include "main.h"
 void LCD_EnablePulse(void)
{
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);

}
void LCD_WriteHighNibble(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    LCD_EnablePulse();
}
void LCD_WriteLowNibble(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    LCD_EnablePulse();
}
void LCD_Command(uint8_t cmd)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET); // RS = 0
    LCD_WriteHighNibble(cmd);
    LCD_WriteLowNibble(cmd);
    HAL_Delay(2);
}
void LCD_Data(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET); // RS = 1
    LCD_WriteHighNibble(data);
    LCD_WriteLowNibble(data);
}
void LCD_Init(void)
{
    HAL_Delay(20);

    LCD_Command(0x33);  // Initialize
    LCD_Command(0x32);  // 4-bit mode
    LCD_Command(0x28);  // 2 line, 5x7 font
    LCD_Command(0x0C);  // Display ON
    LCD_Command(0x01);  // Clear display
    HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET);//once read?write pin is low never modify
        HAL_Delay(1);
}
void LCD_WriteString(char *str)
{
    while (*str)
    {
        if (*str == '\n')
        {
            LCD_Command(0xC0);  // Move to second line
        }
        else
        {
            LCD_Data((uint8_t)(*str));
        }
        str++;
    }
}
void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t address;

    if (row == 0)
        address = 0x80 + col;   // First line
    else
        address = 0xC0 + col;   // Second line

    LCD_Command(address);
}
void LCD_Clear(void)
{
    LCD_Command(0x01);   // Clear display command
    HAL_Delay(2);        // LCD needs >1.53 ms after clear
}
