#include "interrupt.h"
#include "gpio_config.h"

void Interrupt_Config(void){
	NVIC_InitTypeDef nvic;
	EXTI_InitTypeDef exti;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	//Cau hinh NVIC, bo dieu khien ngat
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	nvic.NVIC_IRQChannel = EXTI4_IRQn; // Config bo NVIC cho Button PA4 - Toggle
	nvic.NVIC_IRQChannelCmd = ENABLE; // Cho phep hoac cam ngat ngoai vi dag cau hinh tai NVIC
	nvic.NVIC_IRQChannelPreemptionPriority = 6;
	nvic.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic);
	
	nvic.NVIC_IRQChannel = EXTI3_IRQn; // Config bo NVIC cho Button PA3 - Che do
	nvic.NVIC_IRQChannelCmd = ENABLE; // Cho phep hoac cam ngat ngoai vi dag cau hinh tai NVIC
	nvic.NVIC_IRQChannelPreemptionPriority = 5;
	nvic.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic);
	
	// Config ngat cho Button Toggle PA4
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);
	exti.EXTI_Line = EXTI_Line4;
	exti.EXTI_LineCmd = ENABLE;
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&exti);
	
	// Config ngat cho Button Chedo PA3
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);
	exti.EXTI_Line = EXTI_Line3;
	exti.EXTI_LineCmd = ENABLE;
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&exti);
}

//Ham ngat Button Toggle
void EXTI4_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line4) != RESET){
		
		BaseType_t xHigherPriorityTaskWoken = 0;
		Event_t event = EVENT_TOGGLE_BUTTON;		
		
		static TickType_t s_lastPressTime = 0; 

    TickType_t currentTime = xTaskGetTickCountFromISR(); // Lay thoi gian hien tai
		
		EXTI_ClearITPendingBit(EXTI_Line4);
		
    if ((currentTime - s_lastPressTime) < pdMS_TO_TICKS(500))
    {
        return; // Thoát ngay
    }
		
		s_lastPressTime = currentTime;
		
		xQueueSendFromISR(xControlQueue, &event, &xHigherPriorityTaskWoken);
		
		
		
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

//Ham ngat Button Mode
void EXTI3_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line3) != RESET){
		
		BaseType_t xHigherPriorityTaskWoken = 0;
		Event_t event = EVENT_MODE_BUTTON;		
		
		static TickType_t s_lastPressTime_Mode = 0; 
    TickType_t currentTime = xTaskGetTickCountFromISR();
		
		EXTI_ClearITPendingBit(EXTI_Line3);
		
    if ((currentTime - s_lastPressTime_Mode) < pdMS_TO_TICKS(500))
    {
        return;
    }
		
		s_lastPressTime_Mode = currentTime;
		
		xQueueSendFromISR(xControlQueue, &event, &xHigherPriorityTaskWoken);
		
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}