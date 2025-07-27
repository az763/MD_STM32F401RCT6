#ifndef __KT0803L_SOFTWARE_I2C_H
#define __KT0803L_SOFTWARE_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

//#include "main.h"
#include "math.h"
#include "stdio.h"
//#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_ll_gpio.h"

// In CUBE MX Configure the GPIO Pins to Output
// HAL 
//#define SCL_I HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);// PA4 - SCL 
//#define SDA_I HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);// PA5 - SDA 
//#define SCL_O HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
//#define SDA_O HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

// LL / CMSIS 
#define KT0803L_SCL_I LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_4)// PA4 - SCL 
#define KT0803L_SDA_I LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_5)// PA5 - SDA 
#define KT0803L_SCL_O (GPIOA->BSRR = GPIO_BSRR_BR4) 
#define KT0803L_SDA_O (GPIOA->BSRR = GPIO_BSRR_BR5) 

// List of supported speeds:
// Delay=1us, speed=250kHz
// Delay=2us, speed=125kHz
// Delay=3us, speed=83kHz
// Delay=4us, speed=62kHz
// Delay=5us, speed=50kHz
// Delay=6us, speed=41kHz
// Delay=7us, speed=35kHz
// Delay=8us, speed=31kHz
// Delay=9us, speed=27kHz
// Delay=10us, speed=25kHz
// ... delays more than 10us are also possible.
//#define DELAY_TIME_US 10 // 10 us.
#define DELAY_TIME_US ((uint32_t)0x14) // 20 us. OK ?

//--------------------------------------------------------------------------------
void KT0803L_i2c_init(void);               // Инициализация шины
void KT0803L_i2c_start_cond(void);        // Генерация условия старт
void KT0803L_i2c_restart_cond(void);      // Генерация условия рестарт
void KT0803L_i2c_stop_cond(void);        // Генерация условия стоп  
uint8_t KT0803L_i2c_send_byte(uint8_t data);      //Передать байт (вх. аргумент передаваемый байт) (возвращает 0 - АСК, 1 - NACK) 
uint8_t KT0803L_i2c_get_byte(uint8_t last_byte);  //Принять байт (если последний байт то входной аргумент = 1, если будем считывать еще то 0)(возвращает принятый байт)
//--------------------------------------------------------------------------------
// KT0803L The I2C write address is 0x7C and the read address is 0x7D. 
#define KT0803L_CMD_WRITE 0x7C 
#define KT0803L_CMD_READ 0x7D 
//--------------------------------------------------------------------------------

/*
//#define KT0803L_REGS_MAX 18 
// Value default 
const uint8_t Reg_00_Default_value = 0x5C;// 0b01011100
const uint8_t Reg_01_Default_value = 0xC3;// 0b11000011
const uint8_t Reg_02_Default_value = 0x40;// 0b01000000
const uint8_t Reg_04_Default_value = 0x04;// 0b00000100

const uint8_t Reg_0B_Default_value = 0x00;// 0b00000000
const uint8_t Reg_0C_Default_value = 0x00;// 0b00000000 New
const uint8_t Reg_0E_Default_value = 0x02;// 0b00000010
volatile uint8_t Reg_0F_Read_only;// Только чтение

const uint8_t Reg_10_Default_value = 0xA8;// 0b10101000
const uint8_t Reg_12_Default_value = 0x80;// 0b10000000
const uint8_t Reg_13_Default_value = 0x80;// 0b10000000
const uint8_t Reg_14_Default_value = 0x00;// 0b00000000

const uint8_t Reg_15_Default_value = 0xE0;// 0b11100000 New
const uint8_t Reg_16_Default_value = 0x00;// 0b00000000
const uint8_t Reg_17_Default_value = 0x00;// 0b00000000 New
const uint8_t Reg_1E_Default_value = 0x00;// 0b00000000 New

const uint8_t Reg_26_Default_value = 0xA0;// 0b10100000 New
const uint8_t Reg_27_Default_value = 0x00;// 0b00000000 New
//
//const uint16_t CHSEL_Cannel = 2160;// 108 MHz. '0b100001110000' - 12 bits. Target frequency in MHz x 20. 
//const uint16_t CHSEL_Cannel_def = 1720;// 86 MHz. '0b11010111000' - 11 bits.
*/

//--------------------------------------------------------------------------------
void KT0803L_WriteRegister(uint8_t reg_address, uint8_t value);
uint8_t KT0803L_ReadRegister(uint8_t reg_address);
void KT0803L_SetFrequency(float freq, uint8_t PA_enable);// PA_enable 0 - Disable 1 - Enable 
void KT0803L_setPA_BIAS(uint8_t PA_enable);// PA_enable 0 - Disable 1 - Enable 
float KT0803L_GetFrequency();
float KT0803L_ChannelToFrequency(uint16_t channel);
uint16_t KT0803L_FrequencyToChannel(float freq);
void KT0803L_setMute(uint8_t mute);
uint8_t KT0803L_getMute();
void KT0803L_setRFGain(uint8_t rfgain);
uint8_t KT0803L_getRFGain();
void KT0803L_setPGA(uint8_t pga);
uint8_t KT0803L_getPGA();
// PHTCNST Pre-emphasis Time Constant: 1 - 50 us (Europe, Australia), 0 - 75 us (USA, Japan). 
// Default PHTCNST 0 - 75 us (USA, Japan). 
void KT0803L_setPHTCNST(uint8_t value);
uint8_t KT0803L_getPHTCNST();
void KT0803L_setPilotToneAdjust(uint8_t mode);
uint8_t KT0803L_getPilotToneAdjust();
void KT0803L_setMono();
void KT0803L_setStereo();
uint8_t KT0803L_isStereo();
void KT0803L_setBass(uint8_t bass);
uint8_t KT0803L_getBass();
uint8_t KT0803L_powerOK();
uint8_t KT0803L_isSilenceDetected();
void KT0803L_SilenceDetectionDisable();
// Main Tunings 
void KT0803L_Set_Main_Tunings();
// Test
//void KT0803L_Test01();
//--------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif // __KT0803L_SOFTWARE_I2C_H
