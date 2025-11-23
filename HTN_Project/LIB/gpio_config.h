#ifndef _gpio_config_h_
#define _gpio_config_h_

#include "main.h"
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC

#define PIN_BUTTON_MODE GPIO_Pin_3
#define PORT_BUTTON_MODE GPIOA

#define PIN_BUTTON_MANUAL GPIO_Pin_4
#define PORT_BUTTON_MANUAL GPIOA

#define PIN_FAN GPIO_Pin_8
#define PORT_FAN GPIOA

#define PIN_DHT11 GPIO_Pin_0
#define PORT_DHT11 GPIOA

#define PIN_MQ9 GPIO_Pin_1
#define PORT_MQ9 GPIOA

//void Button_Mode_Config(void);
//void Button_Manual_Config(void);
//void Fan_Config(void);
void GPIOA_All_Config(void);
void DHT11_Config(GPIOMode_TypeDef mode);
//void MQ9_Config(void);

#endif
