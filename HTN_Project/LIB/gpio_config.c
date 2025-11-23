#include "gpio_config.h"

//void Button_Mode_Config(void){
//	GPIO_InitTypeDef btn;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//	
//	btn.GPIO_Mode = GPIO_Mode_IPU;
//	btn.GPIO_Pin = PIN_BUTTON_MODE;
//	
//	GPIO_Init(PORT_BUTTON_MODE, &btn);
//}

//void Button_Manual_Config(void){
//	GPIO_InitTypeDef btn;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//	
//	btn.GPIO_Mode = GPIO_Mode_IPU;
//	btn.GPIO_Pin = PIN_BUTTON_MANUAL;
//	
//	GPIO_Init(PORT_BUTTON_MANUAL, &btn);
//}

//void Fan_Config(void){
//	GPIO_InitTypeDef fan;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//	
//	fan.GPIO_Mode = GPIO_Mode_AF_PP;
//	fan.GPIO_Pin = PIN_FAN;
//	fan.GPIO_Speed = GPIO_Speed_50MHz;
//	
//	GPIO_Init(PORT_FAN, &fan);
//}
void GPIOA_All_Config(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    // PA1 (MQ9) - Analog Input
    gpio.GPIO_Pin = PIN_MQ9; // Pin 1
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(PORT_MQ9, &gpio); // PORT_MQ9 là GPIOA
    
    // PA3 (Button Mode) - Input Pull-Up
    gpio.GPIO_Pin = PIN_BUTTON_MODE; // Pin 3
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(PORT_BUTTON_MODE, &gpio); // PORT_BUTTON_MODE là GPIOA
    
    // PA4 (Button Manual) - Input Pull-Up
    gpio.GPIO_Pin = PIN_BUTTON_MANUAL; // Pin 4
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(PORT_BUTTON_MANUAL, &gpio); // PORT_BUTTON_MANUAL là GPIOA
    
    // PA8 (PWM Fan) - Alternate Function Push-Pull
    gpio.GPIO_Pin = PIN_FAN; // Pin 8
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PORT_FAN, &gpio); // PORT_FAN là GPIOA
    
    // PA9 (UART TX) - Alternate Function Push-Pull
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
    
    // PA10 (UART RX) - Input Floating
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);
}

void DHT11_Config(GPIOMode_TypeDef mode){
	GPIO_InitTypeDef GPIO_DHT11;
	
	GPIO_DHT11.GPIO_Mode = mode;
	GPIO_DHT11.GPIO_Pin = PIN_DHT11;
	GPIO_DHT11.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(PORT_DHT11, &GPIO_DHT11);
}

//void MQ9_Config(void){
//	GPIO_InitTypeDef GPIO_MQ9;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//	
//	GPIO_MQ9.GPIO_Mode = GPIO_Mode_AIN;
//	GPIO_MQ9.GPIO_Pin = PIN_MQ9;
//	GPIO_MQ9.GPIO_Speed = GPIO_Speed_50MHz;
//	
//	GPIO_Init(PORT_MQ9, &GPIO_MQ9);
//}

