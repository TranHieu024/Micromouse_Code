#ifndef _main_h_
#define _main_h_

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "queue.h"                      // ARM.FreeRTOS::RTOS:Core
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core
#include "gpio_config.h"
#include "uart.h"
#include "interrupt.h"
#include "dht11.h"
#include "dma.h"
#include "adc.h"
#include "LCD.h"
#include "pwm.h"

typedef enum {
    MODE_AUTO,
    MODE_MANUAL
} FanMode_t;

//Events
typedef enum {
    EVENT_MODE_BUTTON,
    EVENT_TOGGLE_BUTTON,
    EVENT_SENSOR_UPDATE
} Event_t;

extern QueueHandle_t xControlQueue;     // Queue trung tâm
extern FanMode_t Mode;
extern uint8_t Manual;
#endif
