#include "adc.h"

void ADC_Config(void){
	RCC -> APB2ENR |= (1 << 9); // Bat clock cho ADC1 (9)
	
	ADC1 -> CR2 |= 1; // ADC on
	Delay_1ms();
	ADC1 -> CR2 |= (1 << 1) | (1 << 8); // Bat DMA
	ADC1 -> SMPR2 |= (0x4 << 3); // 41.5 cycles cho kênh 1
	// Hieu chinh ADC
	ADC1 -> CR2 |= (1 << 2); // Bat bit CAL (bit 2) len 1 de hieu chinh ADC 
	while(ADC1 -> CR2 & (1 << 2)); // Cho hieu chuan ADC
	
	ADC1 -> SQR3 = 1; // Chon kenh 1 la dau ra thong thuong (thu tu uu tien)
	ADC1 -> CR2 |= 1; // ADC on
	Delay_1ms();
	
	ADC1->CR2 |= (1 << 22); // Start conversion
}

