#ifndef _dht11_h_
#define _dht11_h_

#include "gpio_config.h"
#include "delay.h"

extern uint8_t dht11_data[5]; 

void DHT11_Start(void);
uint8_t DHT11_Check_Response(void);
uint8_t DHT11_Read_Bit(void);
uint8_t DHT11_Read_Byte(void);
uint8_t DHT11_Read(void);

#endif
