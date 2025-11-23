#include "pwm.h"

void PWM_Config(void){
			TIM_TimeBaseInitTypeDef tim_init;
			TIM_OCInitTypeDef oc_init;
			
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

			// 3. Cau hình Time Base (f = 1 kHz)
			tim_init.TIM_Prescaler = 71;          // PSC = 71 -> clk = 72MHz/72 = 1 MHz
			tim_init.TIM_Period = 999;            // ARR = 999 -> T = 1000 tick = 1 ms = 1 kHz
			tim_init.TIM_CounterMode = TIM_CounterMode_Up;
			TIM_TimeBaseInit(TIM1, &tim_init);

			// 4. Cau hình PWM cho CH1
			oc_init.TIM_OCMode = TIM_OCMode_PWM1;

			oc_init.TIM_OutputState = TIM_OutputState_Enable;
			oc_init.TIM_Pulse = 0;              // CCR1 = 300 -> Duty = 300/1000 = 30% (if connect common +)
																						// if connect common - then 100% - duty (30%)
			oc_init.TIM_OCPolarity = TIM_OCPolarity_High;
			TIM_OC1Init(TIM1, &oc_init);
			TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

			// 5. Bat output chính (MOE) vì TIM1 là advanced timer
			TIM_CtrlPWMOutputs(TIM1, ENABLE);

			// 6. Start Timer
			TIM_Cmd(TIM1, ENABLE);
}

void SetDuty(uint8_t percent){
    if (percent > 100) percent = 100;
    
    // 5. Gán giá tri vào thanh ghi Compare 3
    TIM1->CCR1 = (percent * 10); 
}