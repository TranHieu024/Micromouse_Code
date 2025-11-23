
#include "LCD.h"
#include "delay.h"

// Khoi tao I2C1 giao tiep voi LCD8574
void I2C_Config(void) {
    GPIO_InitTypeDef GPIO_InitStruct2;
    I2C_InitTypeDef I2C_InitStruct;
    
    // Cap xung clock cho I2C1 va GPIOB
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // Cau hinh chan PB6 (SCL) va PB7 (SDA) che do thay the open drain
    GPIO_InitStruct2.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct2.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStruct2.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct2); 
    
    // Cau hinh I2C o che do master, dia chi cua no la 0x00
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C; //Cau hinh che do hoat dong
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2; // Cau hinh ti le xung cua SCL
    I2C_InitStruct.I2C_OwnAddress1 = 0x00; //Ðia chi cua STM32 khi làm slave (o dây không dùng den vì ta làm master).
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable; //Cho phép ACK sau moi byte nhan
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; //Dung kieu dia chi 7 bit
    I2C_InitStruct.I2C_ClockSpeed = 100000; // toc do 100kHz
    I2C_Init(I2C1, &I2C_InitStruct);
    I2C_Cmd(I2C1, ENABLE); // Kich hoat I2C1
}

// Gui 1 byte du lieu den dia chi I2C
void I2C_WriteByte(uint8_t addr, uint8_t data) {
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)); // Doi I2C san sang
    
    I2C_GenerateSTART(I2C1, ENABLE);  // Tao xung START
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter); // Gui dia chi thiet bi
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    
    I2C_SendData(I2C1, data); // Gui du lieu
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
    I2C_GenerateSTOP(I2C1, ENABLE); // Tao xung STOP ket thuc
}

// Khoi tao LCD
void LCD_Init(void) {
    Delay_ms(50);
    LCD_SendCommand(0x28); // Che do 4 bit, 2 dong, font 5x8
    LCD_SendCommand(0x0C); // Bat hien thi, tat con tro
    LCD_SendCommand(0x06); // Tu dong dich con tro
    LCD_SendCommand(0x01); // Xoa man hinh
    Delay_ms(2);
}

// Gui lenh dieu khien den LCD (RS = 0)
void LCD_SendCommand(uint8_t cmd) {
    uint8_t data_u = (cmd & 0xF0) | 0x08; // Lay 4 bit cao, RS = 0
    uint8_t data_l = ((cmd << 4) & 0xF0) | 0x08; // Lay 4 bit thap, RS = 0
    I2C_WriteByte(LCD8574_ADDR, data_u | 0x04);  // En = 1
    I2C_WriteByte(LCD8574_ADDR, data_u);         // En = 0
    I2C_WriteByte(LCD8574_ADDR, data_l | 0x04);  // En = 1
    I2C_WriteByte(LCD8574_ADDR, data_l);         // En = 0
}

// Gui du lieu den LCD (RS = 1)
void LCD_SendData(uint8_t data) {
    uint8_t data_u = (data & 0xF0) | 0x09; // Lay 4 bit cao, RS = 1
    uint8_t data_l = ((data << 4) & 0xF0) | 0x09; // Lay 4 bit thap, RS = 1
    I2C_WriteByte(LCD8574_ADDR, data_u | 0x05); // En = 1
    I2C_WriteByte(LCD8574_ADDR, data_u | 0x01); // En = 0
    I2C_WriteByte(LCD8574_ADDR, data_l | 0x05); // En = 1
    I2C_WriteByte(LCD8574_ADDR, data_l | 0x01); // En = 0
}

// Hien thi chuoi ky tu tren man hinh
void LCD_Print(char *str) {
    while (*str) {
        LCD_SendData(*str++);
    }
}