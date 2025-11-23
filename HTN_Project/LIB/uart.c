#include "uart.h"

char vrc_Getc;
int vri_Stt;
char vrc_Res[MAX];
int vri_Count = 0;

void Uart_Config(void){
	USART_InitTypeDef usart;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	// usart config
	usart.USART_BaudRate = 9600;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength =USART_WordLength_8b;
	USART_Init(USART1, &usart);
	USART_Cmd(USART1, ENABLE);
	
}

//Ham gui 1 ki tu
void Uart_SendChar(char _chr){
	USART_SendData(USART1,_chr);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==0); // Doi co TC len 1 La truyen xong
}

//Gui 1 chuoi string
void Uart_SendStr(char *str){
	while(*str){
		Uart_SendChar(*str++);		
	}
}
