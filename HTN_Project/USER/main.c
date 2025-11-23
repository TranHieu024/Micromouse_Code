#include "stm32f10x.h"                  // Device header
#include "main.h"
#include "stdio.h" // De dung sprintf

// --- INCLUDE CAC DRIVER (De dam bao kieu du lieu) ---
#include "delay.h"
#include "uart.h"
#include "LCD.h"
#include "adc.h"
#include "dma.h"
#include "dht11.h"
#include "gpio_config.h"
#include "pwm.h"

// --- DINH NGHIA ---
#define DS1307_ADDR 0x68

// Trang thai he thong
typedef enum {
    STATE_DASHBOARD = 0, // Man hinh chinh (Cam bien)
    STATE_MENU      = 1  // Man hinh Menu
} SystemState_t;

// --- BIEN TOAN CUC ---
QueueHandle_t xControlQueue;     // Queue nhan nut bam tu ngat
//SemaphoreHandle_t xSmp_DMA;      // Semaphore cho DMA
SemaphoreHandle_t xMutex;        // Mutex bao ve LCD/I2C

FanMode_t Mode = MODE_AUTO;
uint8_t Manual = 0;

// Bien chia se giua cac Task
volatile SystemState_t CurrentState = STATE_DASHBOARD;
volatile int8_t menu_cursor = 0; // 0: Back, 1: Tat Quat, 2: Bat Quat

// =============================================================
// 1. HAM HO TRO PHAN CUNG
// =============================================================

// Cau hinh Encoder PA5, PA6 (Input Pull-Up)
void Encoder_Config_PA5_PA6(void) {
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    gpio.GPIO_Mode = GPIO_Mode_IPU; 
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
}

// Ham xoa man hinh an toan cho RTOS
void LCD_Clear_Task(void) {
    LCD_SendCommand(0x01);
    vTaskDelay(pdMS_TO_TICKS(3)); 
}

// Ham doc RTC (Dung chung I2C1)
uint8_t BCD2DEC(uint8_t data) { return (data >> 4) * 10 + (data & 0x0F); }

void RTC_Read_Safe(uint8_t *h, uint8_t *m, uint8_t *s) {
    // Ham nay phai duoc goi khi da co Mutex
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1, DS1307_ADDR << 1, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2C1, 0x00);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1, DS1307_ADDR << 1, I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    *s = BCD2DEC(I2C_ReceiveData(I2C1) & 0x7F);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    *m = BCD2DEC(I2C_ReceiveData(I2C1));
    I2C_AcknowledgeConfig(I2C1, DISABLE); // NACK byte cuoi
    I2C_GenerateSTOP(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    *h = BCD2DEC(I2C_ReceiveData(I2C1));
    I2C_AcknowledgeConfig(I2C1, ENABLE);
}

// =============================================================
// 2. CAC TASK RTOS
// =============================================================

// --- TASK 1: DOC ENCODER (Priority Cao Nhat - 3) ---
void Task_Encoder(void *pvParameters) {
    uint8_t last_CLK, cur_CLK;
    last_CLK = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);

    for (;;) {
        cur_CLK = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);
        if (cur_CLK != last_CLK && cur_CLK == 0) {
            // Chi cho phep di chuyen con tro khi dang o trong MENU
            if (CurrentState == STATE_MENU) {
                if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) != cur_CLK) {
                    menu_cursor--; 
                } else {
                    menu_cursor++;
                }
                // Gioi han 0->2
                if (menu_cursor > 2) menu_cursor = 0;
                if (menu_cursor < 0) menu_cursor = 2;
            }
        }
        last_CLK = cur_CLK;
        vTaskDelay(pdMS_TO_TICKS(2)); // Polling 2ms
    }
}

// --- TASK 2: DIEU KHIEN & LOGIC (Priority Trung Binh - 2) ---
void Task_Control(void *pvParameters){
    Event_t ReceiveEvent;
    for(;;){
        // Nhan tin hieu nut bam tu ngat (Interrupt.c gui vao)
        if(xQueueReceive(xControlQueue, &ReceiveEvent, 0) == pdTRUE){
            if(xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE){
                
                // --- NUT MODE (PA3) ---
                if(ReceiveEvent == EVENT_MODE_BUTTON){
                    // Nut Mode chi co tac dung o man hinh chinh
                    if (CurrentState == STATE_DASHBOARD) {
                        if(Mode == MODE_AUTO){
                            Mode = MODE_MANUAL;
                            Manual = 0; // Mac dinh tat
                        } else {
                            Mode = MODE_AUTO;
                        }
                    }
                }
                
                // --- NUT TOGGLE (PA4) - DUNG LAM NUT "CHON / MENU" ---
                if(ReceiveEvent == EVENT_TOGGLE_BUTTON){
                    
                    if (CurrentState == STATE_DASHBOARD) {
                        // 1. Vao Menu
                        CurrentState = STATE_MENU;
                        menu_cursor = 0; // Reset ve dong dau
                    }
                    else {
                        // 2. Dang o Menu -> Chon chuc nang
                        if (menu_cursor == 0) { // BACK
                            CurrentState = STATE_DASHBOARD;
                        }
                        else if (menu_cursor == 1) { // TAT QUAT
                            Mode = MODE_MANUAL; // Chuyen sang Manual de giu trang thai
                            Manual = 0;
                            SetDuty(0);
                            CurrentState = STATE_DASHBOARD;
                        }
                        else if (menu_cursor == 2) { // BAT QUAT
                            Mode = MODE_MANUAL;
                            Manual = 1;
                            SetDuty(100);
                            CurrentState = STATE_DASHBOARD;
                        }
                    }
                }
                xSemaphoreGive(xMutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// --- TASK 3: HIEN THI & CAM BIEN (Priority Thap - 1) ---
void Task_Sensor(void *pvParameters){
    char buf[17];
    uint8_t h, m, s;
    
    // Init LCD lan dau (Phai co Delay truoc do de nguon on dinh)
    vTaskDelay(pdMS_TO_TICKS(100));
    if(xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE){
        LCD_Init();
        LCD_Clear_Task();
        LCD_SendCommand(0x80); LCD_Print(" SYSTEM READY ");
        xSemaphoreGive(xMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));

    while(1){
        // --- A. XU LY MAN HINH CHINH ---
        if (CurrentState == STATE_DASHBOARD) {
            
            // 1. Doc Cam Bien (Chi doc khi o Dashboard de tiet kiem thoi gian)
            
            // DHT11 (Tat ngat de doc chinh xac)
            ADC_Cmd(ADC1, DISABLE); // Tat ADC de giam nhieu
            taskENTER_CRITICAL();
            DHT11_Read(); // Du lieu luu vao dht11_data[]
            taskEXIT_CRITICAL();
            ADC_Cmd(ADC1, ENABLE);

            // Gas (MQ9) - Dung DMA
            // Khong can lenh gi, DMA tu dong cap nhat vao bien 'dest'
            
            // 2. Hien thi LCD
            if(xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE){
                
                // Clear man hinh (Dung 0x01 hoi cham, nen overwrite bang khoang trang tot hon)
                // O day ta dung Clear cho don gian code
                LCD_Clear_Task();

                // Dong 1: Ngay gio hoac Mode
                LCD_SendCommand(0x80);
                if (Mode == MODE_AUTO) {
                    RTC_Read_Safe(&h, &m, &s);
                    sprintf(buf, "%02d:%02d AUTO D%d", h, m, dht11_data[0]); // Gio:Phut CheDo DoAm
                    LCD_Print(buf);
                } else {
                    sprintf(buf, "MANUAL Fan:%s ", (Manual)?"ON ":"OFF");
                    LCD_Print(buf);
                }

                // Dong 2: Nhiet do - Gas
                LCD_SendCommand(0xC0);
                sprintf(buf, "T:%dC Gas:%d  ", dht11_data[2], dest[0]);
                LCD_Print(buf);

                // 3. Logic Dieu Khien Quat (Auto Mode)
                if(Mode == MODE_AUTO){
                    if(dht11_data[2] > 35 || dht11_data[0] > 80 || dest[0] > 2500){
                        SetDuty(100);
                    }
                    else if(dht11_data[2] > 33 || dht11_data[0] > 70 || dest[0] > 1800){
                        SetDuty(70);
                    }
                    else if(dht11_data[2] > 30 || dht11_data[0] > 60 || dest[0] > 1500){
                        SetDuty(50);
                    }
                    else{
                        SetDuty(0);
                    }
                }
                // Manual Mode da duoc xu ly o Task_Control

                xSemaphoreGive(xMutex);
            }
        }
        
        // --- B. XU LY MAN HINH MENU ---
        else if (CurrentState == STATE_MENU) {
            if(xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE){
                // Hien thi Menu
                LCD_SendCommand(0x80); 
                LCD_Print(" MENU CAI DAT:  ");
                
                LCD_SendCommand(0xC0);
                switch (menu_cursor) {
                    case 0: LCD_Print("> 1. BACK       "); break;
                    case 1: LCD_Print("> 2. TAT QUAT   "); break;
                    case 2: LCD_Print("> 3. BAT QUAT   "); break;
                }
                xSemaphoreGive(xMutex);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(300)); // Cap nhat moi 300ms
    }
}

// =============================================================
// 3. MAIN FUNCTION
// =============================================================
int main(){ 
    // 1. Config Priority Group (Bat buoc cho STM32 FreeRTOS)
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // 2. Init Hardware
    GPIOA_All_Config();
    Tim_config(); // Timer cho delay
    Uart_Config();  
    Uart_SendStr("System Booting...\n\r");
    
    I2C_Config();   
    
    PWM_Config();
    SetDuty(0);

    ADC_Config();
    DMA_Config();
    
    Interrupt_Config(); // Ngat nut bam
    
    Encoder_Config_PA5_PA6(); // Init Encoder

    // 3. Create RTOS Objects
    xMutex = xSemaphoreCreateMutex();
    xControlQueue = xQueueCreate(10, sizeof(Event_t));
    xSmp_DMA = xSemaphoreCreateBinary();
    
    // 4. Create Tasks
    // Task Encoder chay nhanh nhat de bat xung kip thoi
    xTaskCreate(Task_Encoder, "Encoder", 128, NULL, 3, NULL);
    
    // Task Control xu ly logic va nut bam
    xTaskCreate(Task_Control, "Control", 128, NULL, 2, NULL);
    
    // Task Sensor hien thi LCD va doc cam bien (Stack 256 cho an toan)
    xTaskCreate(Task_Sensor, "Sensor", 256, NULL, 1, NULL);
    
    // 5. Start
    Uart_SendStr("Start Scheduler...\n\r");
    vTaskStartScheduler();
    
    while(1){} // Khong bao gio chay den day
}