#ifndef _delay_h_
#define _delay_h_

#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"

void Tim_config(void);
void Delay_1ms(void);
void Delay_ms(uint16_t time);
void Delay_1us(void);
void Delay_us(uint16_t time);
#endif
