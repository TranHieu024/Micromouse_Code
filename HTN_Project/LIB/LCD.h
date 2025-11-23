#ifndef _LCD_H_
#define _LCD_H_

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#include "stdio.h"

#define LCD8574_ADDR 0x4E //Dia chi cua I2C la 0x27, tuy nhien o day ta phai dich 1 bit

void I2C_Config(void);
void I2C_WriteByte(uint8_t addr, uint8_t data);
void LCD_Init(void); 
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_Print(char *str);

#endif
