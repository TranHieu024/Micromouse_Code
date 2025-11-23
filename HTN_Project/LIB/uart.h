#ifndef _uart_h
#define _uart_h

#include "stm32f10x_usart.h"
#include "stdio.h"
#include "string.h"
#include "gpio_config.h"
#define MAX 100

extern char vrc_Getc;
extern int vri_Stt;
extern char vrc_Res[MAX];	
extern int vri_Count;

void Uart_Config(void);
void Uart_SendChar(char _chr);
void Uart_SendStr(char *str);
#endif
