#ifndef _pwm_h_
#define _pwm_h_

#include "stm32f10x_tim.h"

void PWM_Config(void);
void SetDuty(uint8_t percent);

#endif
