#include "delay.h"

void Tim_config(void){
	TIM_TimeBaseInitTypeDef timer;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	timer.TIM_Period = 65535;
	timer.TIM_Prescaler = 2 - 1;
	
	TIM_TimeBaseInit(TIM3, &timer);
}

void Delay_1ms(void){
	TIM_Cmd(TIM3, ENABLE);
	TIM_SetCounter(TIM3, 0);
	while(TIM_GetCounter(TIM3) < 36000);
	TIM_Cmd(TIM3, DISABLE);
}

void Delay_ms(uint16_t time){
	while(time--) Delay_1ms();
}

void Delay_1us(void){
	TIM_Cmd(TIM3, ENABLE);
	TIM_SetCounter(TIM3, 0);
	while(TIM_GetCounter(TIM3) < 36);
	TIM_Cmd(TIM3, DISABLE);
}

void Delay_us(uint16_t time){
	while(time--) Delay_1us();
}