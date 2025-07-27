#ifndef __SOFTWARE_I2C_H
#define __SOFTWARE_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

// G1KuL1N
// ПОДКЛЮЧИТЬ БИБЛИОТЕКУ СВОЕГО КОНТРОЛЛЕРА

//#include "stm32f4xx_hal_gpio.h"
//#include "main.h" 
#include "stm32f4xx_ll_gpio.h"

// В CUBE MX порты I2C настроить на выход (либо в main.c вручну подать тактирование на нужны GPIO) 

//---подключение шины к пинам-----------------------------------------------------
//#define SCL_I HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);// PC15 - SCL 
//#define SDA_I HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14);// PC14 - SDA 
//#define SCL_O HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
//#define SDA_O HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);

// LL / CMSIS
#define SCL_I LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_15)// PC15 - SCL 
#define SDA_I LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_14)// PC14 - SDA 
#define SCL_O (GPIOC->BSRR = GPIO_BSRR_BR15) 
#define SDA_O (GPIOC->BSRR = GPIO_BSRR_BR14) 

//--------------------------------------------------------------------------------
void i2c_init(void);               // Инициализация шины
void i2c_start_cond(void);        // Генерация условия старт
void i2c_restart_cond(void);      // Генерация условия рестарт
void i2c_stop_cond(void);        // Генерация условия стоп  
uint8_t i2c_send_byte(uint8_t data);      //Передать байт (вх. аргумент передаваемый байт) (возвращает 0 - АСК, 1 - NACK) 
uint8_t i2c_get_byte(uint8_t last_byte);  //Принять байт (если последний байт то входной аргумент = 1, если будем считывать еще то 0)(возвращает принятый байт)
//--------------------------------------------------------------------------------
// ПРИМЕР ИСПОЛЬЗОВАНИЯ
//=========================================================================================
//   Запись uint16_t во внешнюю еепром (FRAM FM24CL64) или любой другой 24LC памяти, 
//	 две ячейки,  указывается адрес первой ячейки, следующая идет adr++
//=========================================================================================
//
//void FRAM_W_INT(uint16_t adr, uint16_t dat){
//i2c_start_cond ();
//i2c_send_byte (0xA2); //адрес чипа + что будем делать (записывать)
//i2c_send_byte    ((adr & 0xFF00) >> 8);  
//i2c_send_byte    (adr & 0x00FF);
//i2c_send_byte    ((dat & 0xFF00) >> 8);  
//i2c_send_byte    (dat & 0x00FF);
//i2c_stop_cond();
//}

//=========================================================================================
//   Считывание uint16_t из внешней еепром (FRAM FM24CL64) или любой другой 24LC памяти, 
//	 две ячейки,  указывается адрес первой ячейки, следующая идет adr++
//=========================================================================================
//uint16_t FRAM_R_INT(uint16_t adr){
//uint16_t dat;
//i2c_start_cond ();
//i2c_send_byte (0xA2);
//i2c_send_byte    ((adr & 0xFF00) >> 8);  
//i2c_send_byte    (adr & 0x00FF);
//i2c_restart_cond ();
//i2c_send_byte (0xA3);
//dat =  i2c_get_byte(0);	
//dat <<= 8; 
//dat |= i2c_get_byte(1);
//i2c_stop_cond();
//return dat;
//}

// G1KuL1N

//------------------------------------------------
// This source code modified by: Aleksandr Z.

#define I2C_REQUEST_WRITE 0x00
#define I2C_REQUEST_READ  0x01
#define SLAVE_OWN_ADDRESS 0xA0 // 0xA2

void EEPROM_Write16(uint16_t addr, uint16_t data);
uint16_t EEPROM_Read16(uint16_t addr);

//------------------------------------------------
// Addressing EEPROM
// FM Transmitter
#define EEPROM_ADDR_KT0803L_FM_CANNEL ((uint16_t)0x0010) // FM Transmitter 
#define EEPROM_ADDR_KT0803L_ON_OFF ((uint16_t)0x00B0) // FM Transmitter - ON / OFF
// TX
#define EEPROM_ADDR_FREQ_TX ((uint16_t)0x0020) // TX Frequency 
#define EEPROM_ADDR_GEB ((uint16_t)0x0030) // GEB value 
// RX
#define EEPROM_ADDR_FILTER_SPEED ((uint16_t)0x0040) // Speed index 
#define EEPROM_ADDR_SENSE ((uint16_t)0x0050) // Barrier 
#define EEPROM_ADDR_SW_BUFFER_MODE ((uint16_t)0x0060) // Switch mode buffering 
//#define EEPROM_ADDR_SW_FILTER_MODE ((uint16_t)0x0070) // Switch filter
#define EEPROM_ADDR_RX_MODE ((uint16_t)0x0080) // RX sample bit 
#define EEPROM_ADDR_FERRUM_MASK ((uint16_t)0x0090) // Mask Ferrum mode flag
//#define EEPROM_ADDR_COMPARE_MODE ((uint16_t)0x00A0) // Comparator mode flag

//------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* __SOFTWARE_I2C_H */
