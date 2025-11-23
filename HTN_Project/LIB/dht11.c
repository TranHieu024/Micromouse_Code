#include "dht11.h"

uint8_t dht11_data[5]; 

// Gui tin hieu danh thuc DHT11
void DHT11_Start(void) {
    DHT11_Config(GPIO_Mode_Out_PP);
    GPIO_ResetBits(PORT_DHT11, PIN_DHT11);
    Delay_ms(20);
    GPIO_SetBits(PORT_DHT11, PIN_DHT11);
    Delay_us(30);
    DHT11_Config(GPIO_Mode_IPU);
}
// Kiem tra phan hoi tu dht11 de xac nhan cam bien san sang gui du lieu
uint8_t DHT11_Check_Response(void) {
    uint8_t time = 0;
    while (GPIO_ReadInputDataBit(PORT_DHT11, PIN_DHT11) == 1) {
        if(++time > 90) return 0;
        Delay_1us();
    }

    time = 0;
    while (GPIO_ReadInputDataBit(PORT_DHT11, PIN_DHT11) == 0) {
        if(++time > 90) return 0;
        Delay_1us();
    }

    return 1;
}
//Doc 1 bit du lieu tu dht11
uint8_t DHT11_Read_Bit(void) {
    uint8_t time = 0;
	
		//cho keo xuong 0 de bat dau truyen du lieu 1 bit
    while (GPIO_ReadInputDataBit(PORT_DHT11, PIN_DHT11) == 1 && time < 100) {
        time++;
        Delay_1us();
    }
		
    time = 0;
		//Cho keo len 1 de dem tgian
    while (GPIO_ReadInputDataBit(PORT_DHT11, PIN_DHT11) == 0 && time < 60) {
        time++;
        Delay_1us();
    }
		Delay_us(30);
		if(GPIO_ReadInputDataBit(PORT_DHT11, PIN_DHT11)) return 1; // qua 30us van o muc cao -> bit 1
    else return 0; // duoi 30us -> bit 0
}
// Doc 1 byte = 8 bit du lieu 
uint8_t DHT11_Read_Byte(void){
    uint8_t i, data = 0;
    for(i = 0; i < 8; i++){
        data <<= 1;
        data |= DHT11_Read_Bit();
    }
    return data;
}
// Doc du lieu tu cam bien
uint8_t DHT11_Read(void) {
    uint8_t i, checksum;
    DHT11_Start();
	
    if (DHT11_Check_Response() == 1) {
        for (i = 0; i < 5; i++) {
            dht11_data[i] = DHT11_Read_Byte();
        }
        
        checksum = dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3];
        if (checksum == dht11_data[4]) { // Byte du lieu cuoi = tong check sum -> du lieu dung
            return 1;
        }
				else return 0;
    }
		return 0;
}


