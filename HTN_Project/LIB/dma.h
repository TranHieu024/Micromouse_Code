#ifndef _dma_h_
#define _dma_h_

#include "stm32f10x_dma.h"              // Keil::Device:StdPeriph Drivers:DMA
#include "main.h"
extern SemaphoreHandle_t xSmp_DMA;
extern uint16_t dest[1];

void DMA_Config(void);

#endif
