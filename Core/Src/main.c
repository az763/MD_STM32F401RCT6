/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>// snprintf
#include <string.h>// strlen
#include <math.h>// atan2f

#include "usart.h"
//#include "fir_lowpass_L6.h" // old code
#include "fir_highpass_bessel.h"
#include "button.h"
#include "lcd2004.h"
#include "lcd1602_menu.h"
#include "logo.h"
#include "flash_protect_f401.h"
#include "software_I2C.h"
#include "kt0803l_software_i2c.h"
// end includes

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SYSCLOCK 84000000UL
#define F_TIM1CLK 84000000UL// APB2 timer TIM1 clocks (Hz)
// FPU
#define ENABLE_FPU SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));// set CP10 and CP11 Full Access 

#define NB_SAMPLE 8 // No comments :)
#define NB_SAMPLE_DIV2 (NB_SAMPLE / 2)// 4
#define NB_SAMPLE_MASK (NB_SAMPLE - 1)// 7
#define NB_SAMPLE_DIV2_MASK (NB_SAMPLE_DIV2 - 1)// 3
// Frequency
volatile uint16_t F_TX = 19300;// Hz
//volatile uint16_t F_TX = 17765;// Hz
//
volatile uint16_t F_TX_Max = 31000;//25500;// Max. 25500 Hz.
volatile uint16_t F_TX_Min = 6000;// Min. 6000 Hz.
volatile uint16_t F_TX_Output = 7000;// temp value
uint8_t AutoSearch_fl = 0;// 1 - enable 
// Tmp registers 
__IO uint32_t tmpreg;
__IO uint32_t tmpreg1;
__IO uint32_t tmpreg2;
__IO uint32_t tmpreg3;
//

// PB8 - debug pin
//#define PB8_INV (GPIOB->ODR ^= GPIO_ODR_ODR8)
//#define PB8_SET_BIT (GPIOB->BSRR = GPIO_BSRR_BS8)
//#define PB8_CLR_BIT (GPIOB->BSRR = GPIO_BSRR_BR8)
// LED1 PC13
//#define LED1_INV (GPIOC->ODR ^= GPIO_ODR_ODR_13)
//#define LED1_OFF (GPIOC->BSRR = GPIO_BSRR_BS13)
//#define LED1_ON (GPIOC->BSRR = GPIO_BSRR_BR13)
// Sound pin - PA3 
//#define PA3_INV (GPIOA->ODR ^= GPIO_ODR_ODR_3)
//#define PA3_CLR_BIT (GPIOA->BSRR = GPIO_BSRR_BR3)
// Backlight pin - PA1 
#define BACKLIGHT_ON (GPIOA->BSRR = GPIO_BSRR_BS1)
#define BACKLIGHT_OFF (GPIOA->BSRR = GPIO_BSRR_BR1)
//volatile uint8_t SW_Backlight = 1;
volatile uint8_t SW_Signal_buff = 1;// case 1 - default !!!
volatile uint8_t SW_Filter = 0;// 0-3 Default 0 - 4 Hz.
// Mode - RX bit
volatile uint8_t SW_Mode = 1;// = 0;
// Mask Ferrum flag
volatile uint8_t Mask_Ferrum_flag = 0;
float VDI_Ferrum_max = -60.0f;// -60.0 ?

// TIM1
volatile uint32_t TIM1_CNT_T = 0;
volatile uint16_t TIM1_CNT_Period = 0;
volatile uint16_t TIM1_CNT_Max = 0;
volatile uint16_t TIM1_CNT_Min = 0;
volatile uint16_t TIM1_CNT_Tmp = 0;
// TIM3
//#define PB1_BIT (GPIOB->IDR & (1 << 1))// PB1 - TX Pin - TIM3_CH4
#define PB1_BIT LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_1)// PB1 - TX Pin - TIM3_CH4
// ADC
volatile uint32_t ADC1_value;//volatile uint16_t ADC1_value;
//volatile uint16_t ADC2_value;// I_TX
volatile uint32_t ADC_value;// U_TX
volatile int32_t Sample_value[NB_SAMPLE] = {0};
volatile int32_t Average_value[NB_SAMPLE] = {0};
// Amplitude
volatile uint32_t Amp_Sample_value[NB_SAMPLE] = {0};
volatile uint32_t Amp_Average_value[NB_SAMPLE] = {0};
// Phase coil
//volatile uint32_t Amp_X = 0;
//volatile uint32_t Amp_Y = 0;
volatile int32_t Amp_X = 0;
volatile int32_t Amp_Y = 0;
// Amplitude coil
volatile int32_t Amp_B0_mV = 0;
volatile int32_t Amp_B1_mV = 0;
volatile int32_t Amp_B2_mV = 0;
volatile int32_t Amp_B3_mV = 0;
volatile int32_t Amp_mv = 0;// output mV
// end Amp
volatile int32_t VDI_input[4] = {0};
volatile uint8_t Sample_index = 0;
volatile uint8_t Sample_Ready = 0;// flag
volatile uint8_t VDI_flag = 0;
// toggle
extern uint8_t sw_default;// switch menu
extern volatile uint16_t menu_pos;

// Sampling and processing
#define NB_AVERAGE 32// 3.3V mcp6022 new //32//16// 64 - Maximum Sensitivity
#define NB_AVERAGE_MASK (NB_AVERAGE - 1)// 31
#define NB_AVERAGE_DIV2 (NB_AVERAGE / 2)// 16
#define NB_AVERAGE_DIV4 (NB_AVERAGE / 4)// 8
volatile uint8_t Samples_cnt = 0;
//const float Pi = 3.1415927f;
//const float Radians_To_Degrees = 180.00f / 3.1415927f;
const float Radian = 57.2957786f;
volatile float VDI_value = 0.00f;
// Target
volatile int32_t Target_value = 0;
volatile uint8_t Target_flag = 0;
volatile uint8_t Audio_EN = 0;// flag
//volatile uint16_t Audio_cnt = 49;// stm32f103

// SENSE
volatile uint16_t Sense_value = 50;
// BG
volatile float BG = -22.00f;// = 0.00f;
// GEB Fixed = 7. TIM3_CH4 - CH Polarity High.
volatile uint32_t TIM3_GEB_value = (uint32_t)NB_SAMPLE_MASK;// = 7, 0-7 GEB // TIM3->CCR4;
//volatile float BG_Tune_Ferr = 0.00f;// Min: -22.50f Max: +22.50f
//volatile uint8_t Comparator_2_flag = 0;// Comparator 2 flag

// Timer 2
// TIM2 10 us, 100000 tick/sec. Prescaler 84-1, Counter Period 10-1 = 9.
#define TIM2_DIV 10// 10us, TIM2->PSC = 84-1; TIM2->ARR = 10-1;
volatile uint16_t T_2Hz = 50000;// 500 ms DIV 10
volatile uint16_t T_4Hz = 25000; // 250 ms
volatile uint16_t T_5Hz = 20000; // 200 ms
//volatile uint16_t T_8Hz = 12500; // 125 ms
volatile uint16_t T_16Hz = 6250;// 62,5 ms
//volatile uint16_t T_20Hz = 5000;// 50 ms
volatile uint16_t T_24Hz = 4166;// 41,66 ms
//volatile uint16_t T_28Hz = 3571;// 35,71 ms
volatile uint16_t T_32Hz = 3125;// 31,25 ms
volatile uint16_t T_48Hz = 2083;// 20,83 ms
volatile uint16_t T_64Hz = 1562;// 15,62 ms
volatile uint16_t T_H_CNT = 0;
volatile uint16_t T_L_CNT = 0;
// cnt
//volatile uint16_t TIM2_cnt = 0;
volatile uint16_t H_cnt = 0;
volatile uint16_t L_cnt = 0;
volatile uint16_t Calibration_cnt = 0;

// Oversampling in Callback TIM2 
//#define NB_OVERSAMPLE (_N) // _N = 6 Fixed for filter
//#define NB_OVERSAMPLE_DIV2 (NB_OVERSAMPLE / 2)// 3
//#define NB_OVERSAMPLE_MASK (NB_OVERSAMPLE - 1)// 5
//volatile uint8_t RX_Sample_bit = 1;
//volatile uint8_t RX_Sample_bit_R = 3;// = RX_Sample_bit + 2 or +90C
#define RX_Sample_bit 1// + 45 C
#define RX_Sample_bit_R 3// + 135 C
//static uint8_t RX_Sample_bit_offset = 1;
volatile uint8_t RX_Sample_bit_offset = 1;// = 0;
volatile uint8_t RX_bit = 1;// 1 - Default
// TIM2 OVS cnt
volatile uint16_t OVS = 1000;// = 0;
volatile uint16_t OVS_cnt = 0;
volatile uint8_t OVS_nb = 0;
volatile uint16_t Freq_OVS = 100;// Frequency Oversampling 100Hz. 
// Difference
volatile int32_t Ph_diff = 0;
volatile int32_t Amp_diff = 0;
// FIR
volatile int32_t Filter_In = 0;
volatile int32_t Filter_Out = 0;
// v1.0.9 FIR High Pass Bessel Filter 4Hz. 
// Buffer Z-4
volatile int32_t Filter_Out_buff_0 = 0;
volatile int32_t Filter_Out_buff_1 = 0;
volatile int32_t Filter_Out_buff_2 = 0;
volatile int32_t Filter_Out_buff_3 = 0;
volatile int32_t Filter_Out_buff_4 = 0;
//
// Compare Buffer
volatile int32_t Compare_Buffer = 0;
//
//volatile int32_t Filter_tmp = 0;
//volatile uint8_t Filter_ind = 0;// index
//volatile uint8_t Zind = 0;// index
// Calibration
volatile int32_t Calibration_samples[NB_SAMPLE] = {0};
volatile uint8_t Calib_flag = 1;
char str_wait[] = {"Please wait..."};
char str_01[16+1] = {0};
char str_02[16+1] = {0};
volatile uint8_t Calib_auto = 1;// Auto Calibration Enable by default
//volatile uint8_t Calib_auto = 0;// Auto Calibration Disable by default
// log uart
volatile uint8_t Uart_LogLevel = 1;// = 0;
// Filter Speed
volatile uint8_t Speed_Ind = 2;// 32Hz


// Flags UID
//volatile uint8_t UID_0_OK;
//volatile uint8_t UID_1_OK;
//volatile uint8_t UID_2_OK;
//volatile uint8_t UID_3_OK;

// DWT
#define    DWT_CYCCNT    *(volatile uint32_t*)0xE0001004
#define    DWT_CONTROL   *(volatile uint32_t*)0xE0001000
#define    SCB_DEMCR     *(volatile uint32_t*)0xE000EDFC
uint32_t count_tick = 0;
//

// KT0803L FM Transmitter
float KT0803L_Frequency = 97.10f;// default 97,10 MHz - Any value. 
uint16_t KT0803L_Channel = 1942;// default 97,10 MHz x 20. 
uint16_t KT0803L_Channel_tmp = 1942;
uint8_t PA_BIAS_enable = 0;// = 1;
uint16_t KT0803L_ON_flag = 1;// 1 - ON, 0 - OFF. Enable and disable chip KT0803L. 
// PB2 connected to pin SW1 KT0803L. The defines in main.h 
//#define KT0803L_ON (GPIOB->BSRR = GPIO_BSRR_BS2)
//#define KT0803L_OFF (GPIOB->BSRR = GPIO_BSRR_BR2)
//

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM5_Init(void);
/* USER CODE BEGIN PFP */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
PUTCHAR_PROTOTYPE
{
	USART_TX((uint8_t *)&ch, 1);//HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
	return ch;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


// TIM2 Callback
void TIM2_Callback(void)// T 10 us
{
	// Enable Interrupt
	__enable_irq();
	
	// Oversampling.
	// Oversampling 100 Hz.
	if (OVS_cnt == (uint16_t)OVS){ // OVS = 100000 / 100 = 1000.
		OVS_cnt = 0;
		/*
		// FILTER
		// FIR LowPass Filter. 
		// There is no delay line. The array coefficients are multiplied in reverse order.
		static int32_t Zval[_N + 1];// NB 6 + 1
		//Filter_ind = OVS_nb;// index
		// Z-1 
		//Zval[Filter_ind] = Zval[Filter_ind + 1];// Zval[0] = Zval[1], ..., Zval[15] = Zval[16].
		// ok
		//Zval[OVS_nb] = Zval[OVS_nb + 1];// Zval[0] = Zval[1], ..., Zval[15] = Zval[16]. OK
		Zind = OVS_nb + 1;
		Zval[OVS_nb] = Zval[Zind];// Zval[0] = Zval[1], ..., Zval[15] = Zval[16]. OK
		// apply
		Filter_In = Zval[OVS_nb];//Filter_In = Zval[Filter_ind];
		if (OVS_nb <= (uint8_t)NB_OVERSAMPLE_MASK) {// 0, 1, 2, 3, 4, 5, 6, 7. N = 8. N = 16.
			//Filter_tmp += Filter_In * H_coeff[Filter_ind];// Apply filter
			Filter_tmp += Filter_In * H_coeff[OVS_nb];// Apply filter
		}		
		// Input - the Delay "Z Line -1".
		//Zval[_N] = Average_value[RX_Sample_bit];// N = 4 // bad
		//RX_bit = RX_Sample_bit + RX_Sample_bit_offset;// ok - Value already assigned.
		Zval[_N-1] = Average_value[RX_bit];// N = 3 // OK 
		
		// OVS Index ++
		OVS_nb++;
		// :-) I have nothing to do
		//if(OVS_nb > (uint8_t)NB_OVERSAMPLE_MASK){// if > 15 ERROR or N = 4 // bad
		if(OVS_nb >= (uint8_t)NB_OVERSAMPLE_MASK){// if >= 15 !!!!! N = 3 // OK 
			OVS_nb = 0;
			// Filter divide value
			Filter_Out = (Filter_tmp >> FIXED_POINT_FRACTIONAL_BITS);// Divide by 2^(14)
			Filter_tmp = 0;// clr			
		}
		// End Filter
		*/
		
		// Filter
		Filter_In = Average_value[RX_bit];
		float FirOut = 0.0f;
		switch (SW_Filter) {
			case 0:// 4 Hz
			FirOut = fir_highpass_bessel_4hz((float)Filter_In);// FIR HIGH PASS BESSEL FILTER 4 Hz.
			break;
			case 1:// 8 Hz
			FirOut = fir_highpass_bessel_8hz((float)Filter_In);// FIR HIGH PASS BESSEL FILTER 8 Hz.
			break;
			case 2:// 12 Hz
			FirOut = fir_highpass_bessel_12hz((float)Filter_In);// FIR HIGH PASS BESSEL FILTER 12 Hz.
			break;
			case 3:// No Filter
			FirOut = (float)Filter_In;
			break;
			default:// 4 Hz
			FirOut = fir_highpass_bessel_4hz((float)Filter_In);// FIR HIGH PASS BESSEL FILTER 4 Hz.
			break;
		}
		Filter_Out = (int32_t)FirOut;
		//
	}
	
	
	// T_H_CNT
	if (H_cnt == T_H_CNT){
		H_cnt = 0;
		
		// Signal buffering
		// v1.0.10 - Buffer Z-2 or Z-4
		switch (SW_Signal_buff) {
			case 0:// Z-3 
			Filter_Out_buff_0 = Filter_Out_buff_1;
			Filter_Out_buff_1 = Filter_Out_buff_2;
			Filter_Out_buff_2 = Filter_Out_buff_3;// add buff 3 - v1.0.11
			Filter_Out_buff_3 = Filter_Out;// Target signal
			break;
			case 1:// Z-4
			Filter_Out_buff_0 = Filter_Out_buff_1;
			Filter_Out_buff_1 = Filter_Out_buff_2;
			Filter_Out_buff_2 = Filter_Out_buff_3;
			Filter_Out_buff_3 = Filter_Out_buff_4;
			Filter_Out_buff_4 = Filter_Out;// Target signal
			break;
			default:// Z-4
			Filter_Out_buff_0 = Filter_Out_buff_1;
			Filter_Out_buff_1 = Filter_Out_buff_2;
			Filter_Out_buff_2 = Filter_Out_buff_3;
			Filter_Out_buff_3 = Filter_Out_buff_4;
			Filter_Out_buff_4 = Filter_Out;// Target signal
			break;
		}
		//
		//RX_bit = RX_Sample_bit + RX_Sample_bit_offset;// ok - Value already assigned.
		//Target_value = Average_value[RX_bit];// v1.0.8
		Target_value = Filter_Out;// v1.0.9
		//Ph_diff = Target_value - Filter_Out;// ok v1.0.8
		//Ph_diff = Target_value - Filter_Out_buff_0;// v1.0.9
		Ph_diff = Target_value - Compare_Buffer;// v1.0.10
		// Inversion
		//if (Ph_diff < 0){
		//	Ph_diff = ~Ph_diff + 1;// Inversion
		//}
		
		//Amp_diff = Target_value - Amp_Average_value[RX_Sample_bit];
		//Amp_diff = Target_value - Amp_Average_value[1];
		Amp_diff = Target_value;// send to Menu - Set GEB.
		
		// Signal target
		//if (!Target_flag){ // v1.0.1
			
			// CMP Value - Comparator.
			// Comparator var 1
			//if ( (Target_value > (Filter_Out + Sense_value)) ){ // ok - v1.0.3 and old
			// Comparator var 1.1 - v1.0.4 and v1.0.5
			//if ( (Target_value > (Filter_Out + Sense_value)) || (Target_value < (Filter_Out - Sense_value)) ){ 
			// Comparator var 1 - v1.0.6
			//if ( (Target_value > (Filter_Out_buff_0 + Sense_value)) ){ // v1.0.6
			// Comparator var 1 - v1.0.7
			//if ( (Target_value > (Filter_Out + Sense_value)) ){ // v1.0.7
			// Comparator var 1 - v1.0.9 FIR High Pass Bessel Filter.
			if ( (Target_value > (Filter_Out_buff_0 + Sense_value)) ){ // v1.0.9			
				// Copy to VDI data input - var 1
				//if (Target_flag == 0) {// Re-updating after VDI calculation and LCD display - Stable VDI.
				if ((Target_flag == 0) && (VDI_flag == 0)) {// Re-updating after VDI calculation and LCD display - Stable VDI.
					VDI_input[0] = Average_value[0];
					VDI_input[1] = Average_value[1];
					VDI_input[2] = Average_value[2];
					VDI_input[3] = Average_value[3];
					VDI_flag = 1;
				}
				Target_flag = 1;// TARGET DETECTED !!!
				Calibration_cnt = 0;// clr counter
			}			
			// Comparator var 2
			//if (Comparator_2_flag) {
			//	if (Target_value < (Filter_Out - Sense_value)){
			//		Target_flag = 1;// TARGET DETECTED !!!
			//	}
			//}
		//}		// v1.0.1
	}
	
	// Average value GB - GEB Speed
	if (L_cnt == T_L_CNT){
		L_cnt = 0;
		//if ((Target_flag == 0) && (Calib_auto == 1)){// if (!Target_flag && Calib_auto){
		if ((Calibration_cnt > 20) && (Target_flag == 0) && (Calib_auto == 1)){// if (!Target_flag && Calib_auto){
			// old - Auto Calibration by timer - update every 5Hz.
			// Auto Calibration by timer - update every 20 / 5 = 4 sec.
			//if (Target_flag == 0) {// Re-updating after VDI calculation and LCD display - Stable VDI.
				Calibration_samples[0] = Average_value[0];
				Calibration_samples[1] = Average_value[1];
				Calibration_samples[2] = Average_value[2];
				Calibration_samples[3] = Average_value[3];
				Calibration_cnt = 0;// clr
			//}
		}
		Calibration_cnt++;// 5 Hz
		// update every 4Hz. //
		// Amplitude X,Y
		//Amp_X = Amp_Average_value[RX_Sample_bit];// + 45 degrees
		//Amp_Y = Amp_Average_value[RX_Sample_bit_R];
		// Phase Amp
		//Amp_X = Average_value[RX_Sample_bit];// + 45 degrees
		//Amp_Y = Average_value[RX_Sample_bit_R];// + 135 degrees ?
		// Amplitude to Coil Balance in Millivolt 
		if (menu_pos == 60){// // Menu - Coil balance
			Amp_B0_mV = (int32_t)Amp_Average_value[0];
			Amp_B1_mV = (int32_t)Amp_Average_value[1];
			Amp_B2_mV = (int32_t)Amp_Average_value[2];
			Amp_B3_mV = (int32_t)Amp_Average_value[3];
			// Phase Amp
			Amp_X = Average_value[RX_Sample_bit];// + 45 degrees
			Amp_Y = Average_value[RX_Sample_bit_R];// + 135 degrees ?
		}
		// Update RX bit !!! 5 Hz.
		RX_bit = RX_Sample_bit + RX_Sample_bit_offset;// Update RX bit
		//
		// v1.0.10 Compare Buffer
		if (Target_flag == 0) Compare_Buffer = Filter_Out_buff_0;// if No Signal
		//
	}	
	// cnt
	//TIM2_cnt++;
	H_cnt++;
	L_cnt++;
	OVS_cnt++;
	// cnt max
	//if (TIM2_cnt > Audio_cnt) {//if (TIM3_cnt > 9) {
	//	TIM2_cnt = 0;
	//}
	// AUDIO not use TIM4
	//if ((Audio_EN > 0) && (TIM2_cnt == Audio_cnt)) {//if (Audio_EN && (TIM2_cnt == 9)) {
	//	PA3_INV;
	//	//Audio_EN--;		
	//}// 1 kHz
		
}// end TIM2 Callback


void Timer_Match()
{	
	if (F_TX > F_TX_Max) F_TX = F_TX_Max;// F_TX max.
	if (F_TX < F_TX_Min) F_TX = F_TX_Min;// F_TX min.
	
	TIM1_CNT_T = (uint32_t)((F_TIM1CLK / F_TX) / 2);// 2000 tck
	
	//TIM1_CNT_Period = (uint16_t)((TIM1_CNT_T / NB_SAMPLE) - 1);// TIM1->ARR = 250-1
	TIM1_CNT_Period = (uint16_t)(TIM1_CNT_T / NB_SAMPLE);// 250 tck
	
	// For Auto Tune
	// TIM1_CNT_Max
	TIM1_CNT_T = (uint32_t)((F_TIM1CLK / F_TX_Min) / 2);
	TIM1_CNT_Max = (uint16_t)(TIM1_CNT_T / NB_SAMPLE);//TIM1_CNT_Max = (uint16_t)((TIM1_CNT_T / NB_SAMPLE) - 1);
	printf("\n TIM1_CNT_Max: %lu \n", (uint32_t)TIM1_CNT_Max);
	// TIM1_CNT_Min
	TIM1_CNT_T = (uint32_t)((F_TIM1CLK / F_TX_Max) / 2);
	TIM1_CNT_Min = (uint16_t)(TIM1_CNT_T / NB_SAMPLE);//TIM1_CNT_Min = (uint16_t)((TIM1_CNT_T / NB_SAMPLE) - 1);
	printf("\n TIM1_CNT_Min: %lu \n", (uint32_t)TIM1_CNT_Min);
}

// Restart Timers
void RestartTimers(void)
{
	//Sample_Ready = 0;
	//while (Sample_Ready != 1){ }// Wait Sample_Ready = 1
	//while ((Sample_Ready != 1) && (!PB1_BIT)){ Sample_Ready = 0; }// Wait Sample_Ready = 1 and Wait PB10 Hight level TX pin.
	//
	//while (PB1_BIT){ // Wait Low level TX pin - to correction PHASE TX 
	while (PB1_BIT && (TIM3->CCR4 != 0)){ // Wait Low level TX pin.
		__NOP();
	} 
	__disable_irq();
	LL_TIM_DisableCounter(TIM1);// CLEAR_BIT(TIM1->CR1, TIM_CR1_CEN);
	LL_TIM_DisableCounter(TIM2);// CLEAR_BIT(TIM2->CR1, TIM_CR1_CEN);
	LL_TIM_DisableCounter(TIM3);// CLEAR_BIT(TIM3->CR1, TIM_CR1_CEN);
	LL_mDelay(100);
	// clr counter
	TIM1->CNT = 0;
	TIM2->CNT = 0;
	TIM3->CNT = 0;
	// clr data TIM1 Callback
	Sample_index = 0;
	Samples_cnt = 0;
	//ADC1_value = 0;
	// end clr
	// debug
	//LL_mDelay(100);
	//tmpreg1 = TIM1->CNT;
	//printf("\nRESTART TIM1->CNT = %lu \n", (uint32_t)tmpreg1);
	//
	__enable_irq();
	// Start Timer 1, 2, 3.
	LL_TIM_EnableCounter(TIM1);// SET_BIT(TIM1->CR1, TIM_CR1_CEN);
	LL_TIM_EnableCounter(TIM2);// SET_BIT(TIM2->CR1, TIM_CR1_CEN);
	LL_TIM_EnableCounter(TIM3);// SET_BIT(TIM3->CR1, TIM_CR1_CEN);
	LL_mDelay(500);// Pause 500 ms
	
	// debug
	printf("Restart Timers 1,2,3.\n");
}


// Setup TX Frequency
void SetFrequencyTX(uint16_t TX_freq){// TIM1 Reload counter period
	// set frequency
	F_TX = TX_freq;
	Timer_Match();
	LL_TIM_SetAutoReload(TIM1, (uint32_t)(TIM1_CNT_Period - 1));// auto-reload preload: Enable
	TIM1->CCR1 = (uint32_t)((TIM1_CNT_Period / 2) - 1);// Output compare preload: Enable
	// Restart ???
	RestartTimers();
}


// Auto Tune Frequency
void F_TX_AutoSearch(uint16_t CNT_Min, uint16_t CNT_Max, uint8_t Set_flag)
{	
	volatile uint32_t ADC_value_average = 0;
	volatile int32_t ADC_i32_tmp = 0;
	volatile uint32_t ADC_tmp = 0;
	volatile uint16_t i = 0;	
	
	// Fine tuning
	volatile uint8_t Fine_tuning_flag = 0;
	Fine_tuning: // goto
	
	if (Fine_tuning_flag == 1){
		CNT_Min = TIM1_CNT_Tmp - 20;
		CNT_Max = TIM1_CNT_Tmp + 20;
		ADC_tmp = 0;// clr
	}
	//
	
	/*
	// ADC2
	LL_ADC_Enable(ADC2);
	LL_mDelay(10);// pause
	LL_ADC_StartCalibration(ADC2);// Calibration
	while (LL_ADC_IsCalibrationOnGoing(ADC2) != 0) {};// wait
	*/
	
	printf("Search Frequency CNT_Min %lu CNT_Max %lu\n", (uint32_t)CNT_Min, (uint32_t)CNT_Max);// debug
	
	// Set Frequency minimum !!! 
	LL_TIM_SetAutoReload(TIM1, (uint32_t)(CNT_Max - 1));
	TIM1->CCR1 = (uint32_t)((CNT_Max / 2) - 1);
	LL_mDelay(500);// Pause 500 ms !!!
	//
	
	// LCD
	LCD_Clear();// clr
	
	// Preset cnt
	TIM1_CNT_Tmp = CNT_Max;
	// while
	for(i=CNT_Max; i>CNT_Min; i--){
	
		// WAIT FIXED TIME !!! Correction PHASE.
		while (PB1_BIT && (TIM3->CCR4 != 0)){ // Wait Low level TX pin.
			__NOP();
		} 
		// Get ADC1 Samples value
		//ADC_i32_tmp = (Average_value[1] + Average_value[2]) / 2;// Sample 1,2	ADC1
		// Select Sample
		//ADC_i32_tmp = Average_value[1];// Sample 1 ADC1 - Sample 1 - X // GEB = 3
		ADC_i32_tmp = Average_value[3];// Sample 3 ADC1 - Sample 3 - Y // GEB = 7
		// END FIXED TIME !!!
		if (ADC_i32_tmp < 0) { ADC_i32_tmp = ~ADC_i32_tmp + 1; }// inv		
		ADC_value = (uint32_t)ADC_i32_tmp;
		ADC_value_average = ADC_value;
		
		// LCD
		LCD_SetPos(9, 0);
		LCD_String("       ");
		LCD_SetPos(9, 0);
		sprintf(str_01, "%lu", ADC_value_average);
		LCD_String(str_01);		
		//
		
		// Get period TIM1
		//volatile int32_t XY_val = 10000;// max 10000
		volatile int32_t XY_val = 20000;// max 20000
		
		// var 1
		// CMP Maximum value Sample 1 - X 
		//if((ADC_value_average > ADC_tmp) && (Average_value[3] > 0) && (Average_value[3] < XY_val)){// GEB = 3
		//	TIM1_CNT_Tmp = i;// Set cnt value
		//	ADC_tmp = ADC_value_average;// Set Max value
		//}
		
		// var 2
		// CMP Maximum value Sample 3 - Y 
		if((ADC_value_average > ADC_tmp) && (Average_value[1] > 0) && (Average_value[1] < XY_val)){// GEB = 7		
			TIM1_CNT_Tmp = i;// Set cnt value
			ADC_tmp = ADC_value_average;// Set Max value
		}
		
		
		printf("ADC_value_average: %lu TIM1_CNT_Tmp: %lu\n", (uint32_t)ADC_value_average, (uint32_t)TIM1_CNT_Tmp);// debug
		
		// clr tmp
		ADC_value_average = 0;
		
		// Update Period var 1 - error
		//LL_TIM_SetAutoReload(TIM1, (uint32_t)(TIM1_CNT_Tmp - 1));
		//TIM1->CCR1 = (uint32_t)((TIM1_CNT_Tmp / 2) - 1);
		// Update Period var 2
		LL_TIM_SetAutoReload(TIM1, (uint32_t)(i - 1));
		TIM1->CCR1 = (uint32_t)((i / 2) - 1);
		//LL_mDelay(100);// Pause 100 ms
		//LL_mDelay(50);// Pause 50 ms
		//LL_mDelay(500);// Pause 500 ms
		// OK Pause 50 ms
		LL_mDelay(50);// Pause 50 ms
		
		// if Fine tuning
		if (Fine_tuning_flag == 1){ LL_mDelay(300); } // Pause 300 ms.
		//
		
		// Output frequency tmp
		TIM1_CNT_T = (uint32_t)(i * (uint16_t)NB_SAMPLE * 2);//TIM1_CNT_T = (uint32_t)((i + 1) * NB_SAMPLE * 2);
		F_TX_Output = (uint16_t)(F_TIM1CLK / TIM1_CNT_T);
		printf("\n  Frequency: %lu\n", (uint32_t)F_TX_Output);// debug
		
		// LCD
		LCD_SetPos(0, 0);
		//LCD_String("     ");
		//LCD_SetPos(0, 0);
		sprintf(str_01, "%lu", (uint32_t)F_TX_Output);
		LCD_String(str_01);
		LCD_SetPos(6, 0);
		LCD_String("Hz");
		
		// Output frequency fix
		TIM1_CNT_T = (uint32_t)(TIM1_CNT_Tmp * (uint16_t)NB_SAMPLE * 2);//TIM1_CNT_T = (uint32_t)((TIM1_CNT_Tmp + 1) * NB_SAMPLE * 2);
		F_TX_Output = (uint16_t)(F_TIM1CLK / TIM1_CNT_T);		
		printf("\nF_TX_Output: %lu\n", (uint32_t)F_TX_Output);// debug
		
		// LCD
		LCD_SetPos(0, 1);
		//LCD_String("     ");
		//LCD_SetPos(0, 1);
		sprintf(str_02, "%lu", (uint32_t)F_TX_Output);
		LCD_String(str_02);
		LCD_SetPos(6, 1);
		LCD_String("Hz");
		// end
	}
	
	// Fine tuning
	Fine_tuning_flag += 1;
	
	if (Fine_tuning_flag == 1){ 
		
		LCD_Clear();// clr
		//LCD_SetPos(0, 1);
		LCD_String("Fine tuning");
		LL_mDelay(1000);
		goto Fine_tuning;	
	}
	//
	
	// Offset Frequency -100 Hz!!!
	//F_TX_Output -= 100;// !!!
	
	/*
	// Disable ADC2
	LL_ADC_Disable(ADC2);
	LL_mDelay(10);
	*/
	
	// Update TX Frequency
	if (Set_flag){
		//F_TX = F_TX_Output;
		//Timer_Match();
		//LL_TIM_SetAutoReload(TIM1, (uint32_t)(TIM1_CNT_Period - 1));// auto-reload preload: Enable
		//TIM1->CCR1 = (uint32_t)((TIM1_CNT_Period / 2) - 1);// Output compare preload: Enable
		SetFrequencyTX(F_TX_Output);// TIM1 Reload counter period
		
		printf("\nF_TX Update: %lu\n", (uint32_t)F_TX);// debug
	}
	
	// clr
	TIM1_CNT_Tmp = 0;// volatile
	TIM1_CNT_T = 0;	
}


// For Menu Search Frequency
void Search_Freq(void)
{
	uint8_t fl = 1;
	F_TX_AutoSearch(TIM1_CNT_Min, TIM1_CNT_Max, fl);
}

// WDT get tic
__STATIC_INLINE void DWT_enable()
{
	SCB_DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT_CONTROL |= DWT_CTRL_CYCCNTENA_Msk;
}
__STATIC_INLINE void tic_start()
{
	DWT_CYCCNT = 0;
}
__STATIC_INLINE void tic_end()
{
	// DWT get tic
	count_tick = DWT_CYCCNT;
	printf("\nMCU Tick: %lu \n", count_tick);
}
//

// Get VDI Value 
void GetVDI_Value(void)
{
	//const float f = 1.0;//0.25;// 1.0 / 4.0;
	//float f = 1.00f;
	// ok
	//float bin0 = (float)(VDI_input[0] - Calibration_samples[0]) * f;
	//float bin1 = (float)(VDI_input[1] - Calibration_samples[1]) * f;
	//float bin2 = (float)(VDI_input[2] - Calibration_samples[2]) * f;
	//float bin3 = (float)(VDI_input[3] - Calibration_samples[3]) * f;
	
	// testing ?
	volatile float bin0 = 0.00f;
	volatile float bin1 = 0.00f;
	volatile float bin2 = 0.00f;
	volatile float bin3 = 0.00f;
	// if flag = 1
	if (VDI_flag){
		bin0 = (float)(VDI_input[0] - Calibration_samples[0]);
		bin1 = (float)(VDI_input[1] - Calibration_samples[1]);
		bin2 = (float)(VDI_input[2] - Calibration_samples[2]);
		bin3 = (float)(VDI_input[3] - Calibration_samples[3]);
		VDI_flag = 0;// clr flag !!!
	}
	
	//float amp1 = sqrt((bin0 * bin0) + (bin2 * bin2));
	//float amp2 = sqrt((bin1 * bin1) + (bin3 * bin3));	
	//float amp = (int32_t)((amp1 + amp2) * 0.5f);
	
	//float phase1 = atan2f(bin0, bin2) * Radian + 45.00f;	
	//float phase2 = atan2f(bin1, bin3) * Radian;
	
	volatile float phase1 = atan2f(bin0, bin2) * Radian + 45.00f;	
	volatile float phase2 = atan2f(bin1, bin3) * Radian;
	
	// DWT Counter
	//tic_start();
	//volatile float check_tick = atan2f(bin0, bin2);
	//tic_end();
	//check_tick = check_tick + 1.00f;
	//
	
	// volatile float VDI_value
	//VDI_value = ((phase1 + phase2) * 0.5f) - BG;// +/- phase Adjust 
	VDI_value = ((phase1 + phase2) * 0.5f) + BG;// +/- Phase Adjust 
	
}


// Get Amplitude millivolt
void Get_AMP_mv(float bin0, float bin1, float bin2, float bin3)// bin1, bin3 - 90C.
{
	float U_Vdd = 3210.00f;// millivolt 3.21V
	
	float amp1 = sqrt((bin0 * bin0) + (bin2 * bin2));
	float amp2 = sqrt((bin1 * bin1) + (bin3 * bin3));	
	float Amp_tmp = (amp1 + amp2) * 0.5f;
	
	float Amp_out = (Amp_tmp * U_Vdd) / 4096.00f;
	
	Amp_mv = (int32_t)(Amp_out - (U_Vdd * 0.5f));// offset 1/2. var volatile int32_t Amp_mv;
	
	//if (Amp_mv < 0) { Amp_mv = ~Amp_mv + 1; }
}

// Get Calibration Samples
void MD_StartCalibration(void)
{
	// !!!
	//Sample_Ready = 0;
	//while (Sample_Ready != 1){// Wait Sample_Ready = 1
	//	printf("StartCalibration Wait Sample_Ready: %lu\n", (uint32_t)Sample_Ready);// debug
	//}
	//
	//for (uint8_t i = 0; i < (uint8_t)NB_SAMPLE_DIV2; i++){// Copy 4 Sample
	//	Calibration_samples[i] = Average_value[i];
	//}	
	// Fast copy
	Calibration_samples[0] = Average_value[0];
	Calibration_samples[1] = Average_value[1];
	Calibration_samples[2] = Average_value[2];
	Calibration_samples[3] = Average_value[3];
	// 
	Calib_flag = 0;// clr flag	
	
	// Set input
	VDI_input[0] = Average_value[0];
	VDI_input[1] = Average_value[1];
	VDI_input[2] = Average_value[2];
	VDI_input[3] = Average_value[3];
	//	
	//BG = 0.00;
	GetVDI_Value();
	//
	LCD_Clear();
	//LCD_SetPos(0, 0);
	LCD_String("Calibration OK");
	LCD_SetPos(0, 1);
	char str_02[8];
	sprintf(str_02, "VDI %ld", (int32_t)VDI_value);
	LCD_String(str_02);
	//LCD_SetPos(9, 1);
	//sprintf(str_02, "BG: %ld", (int32_t)BG);
	//LCD_String(str_02);
	LL_mDelay(2000);// Pause 2000 ms
	// end
}

// Setup Ground Balance
void Setup_GB_TIM1_CH1_CNT(float gb, uint8_t gb_flag, uint32_t GEB_CNT_value, uint8_t GEB_flag)
{
	if (gb_flag){
	// Correction Maximum value gb 
	if (gb > 22.50) { gb = 22.50; } // max
	if (gb < -22.50) { gb = -22.50; } // min
	// Match value
	volatile float _1degree_cnt = (float)(TIM1_CNT_Period) / 45.00f;// = 360.00 / NB_SAMPLE
	volatile float GB_degree = _1degree_cnt * gb;// value +/- 
	volatile int16_t GB_CNT = (int16_t)GB_degree;// int16_t value +/- 
	volatile uint16_t TIM1_CH1_CNT = (uint16_t)((TIM1_CNT_Period / 2) - GB_CNT);// Match value cnt
	//debug
	//printf("\n       Input gb %d \n", (int16_t)gb);
	//printf("\n         GB_CNT %d \n", GB_CNT);
	//printf("\nTIM1_CNT_Period %d \n", TIM1_CNT_Period);
	//printf("\n   TIM1_CH1_CNT %d \n", TIM1_CH1_CNT);
	//
	// Correction Maximum and Minimum value CCR1
	if (TIM1_CH1_CNT > TIM1_CNT_Period) { TIM1_CH1_CNT = TIM1_CNT_Period; }
	if (TIM1_CH1_CNT == 0) { TIM1_CH1_CNT = 1; }// Mimimum = 3+1 ???
	// Load value compare in CCR1 TIM1
	// !!! Enable Output compare preload !!!
	TIM1->CCR1 = (uint32_t)(TIM1_CH1_CNT - 1);// Write Reg CCR1 Value - 1
	/* Set the Capture Compare Register value */
	//LL_TIM_OC_SetCompareCH1(TIM1, (uint32_t)(TIM1_CH1_CNT - 1));// Write Reg CCR1 Value - 1
	}
	// Big Step 45C - 1 Sample
	if (GEB_flag){
		// Load value compare in CCR4 TIM3
		// !!! Enable Output compare preload !!!
		// WAIT Low Level TX pin - for load value in TIM3->CCR4
		//while (PB1_BIT){ // Wait Low level TX pin - to correction PHASE TX 
		while (PB1_BIT && (TIM3->CCR4 != 0)){ // Wait Low level TX pin.
			__NOP();
		} 
		__disable_irq();
		switch (GEB_CNT_value) {
			
			// case 0
			case 0:// Disable TIM2 CH3 ?
			TIM3->CCR4 = 0;// Disable TIM3 CH4 ?
			break;
			// case 1
			case 1:// 90C
			TIM3->CCR4 = 1;// = NB_SAMPLE - 7;// = 1;// 2-1;
			break;
			// case 2			
			case 2:// 135C
			TIM3->CCR4 = 2;// = NB_SAMPLE - 6;// = 2;// 3-1;
			break;
			// case 3 - 1/2 Phase 
			case 3://180C
			TIM3->CCR4 = 3;// = NB_SAMPLE - 5;// = 3;// 4-1;
			break;
			// case 4
			case 4:// 225C
			TIM3->CCR4 = 4;// = NB_SAMPLE - 4;// = 4;// = 5-1;
			break;
			// case 5
			case 5:// 270C
			TIM3->CCR4 = 5;// = NB_SAMPLE - 3;// = 5;// = 6-1;
			break;
			// case 6
			case 6:// 315C
			TIM3->CCR4 = 6;// = NB_SAMPLE - 2;// 6;// = 7-1;
			break;
			// case 7
			case 7:// 360C
			TIM3->CCR4 = 7;// = NB_SAMPLE - 1;// TIM3->CCR4 = 7; = NB_SAMPLE_MASK;// = 8-1;
			break;
			// case default
			default:// 360C
			TIM3->CCR4 = 7;// = NB_SAMPLE - 1;// TIM3->CCR4 = 7; = NB_SAMPLE_MASK;// = 8-1;
			//LL_TIM_OC_SetCompareCH4(TIM3, (uint32_t)NB_SAMPLE_MASK);// = 8-1;
			break;
		}
		__enable_irq();
	}
	// Read uint32_t GEB value
	TIM3_GEB_value = TIM3->CCR4;
	
	// Restart Timers TIM1, TIM2, TIM3.
	RestartTimers();
}


/*
// GEB Auto Setup
uint32_t GEB_AutoSetup(void)// Return GEB Value
{
	// LCD info - line 1
	LCD_Clear();
	//LCD_SetPos(0, 0);
	LCD_String("Auto Setup GEB");
	// var
	volatile int32_t Amp_tmp = -1000000;
	volatile int32_t Amp_val = 0;
	volatile uint8_t GEB_index = 0;
	volatile uint8_t GEB_index_out = 0;
	char str_val[8+1] = {0};
	volatile uint8_t i;
	for (i = 0; i < NB_SAMPLE; i++){// i < 8
		// Set GEB = i
		Setup_GB_TIM1_CH1_CNT(0.00, 0, i, 1);// Set GEB = i
		LL_mDelay(100);// Pause 100 ms
		Amp_val = Average_value[RX_Sample_bit];// Get value amplitude
		// debug
		printf("GEB RX Amp_val: %ld\n", (int32_t)Amp_val);
		//		
		if (Amp_val > Amp_tmp){// Select maximum value amplitude
			GEB_index = i;// Save index
			Amp_tmp = Amp_val;// Save old maximum value amplitude
		}
	}		
	//corr GEB_index_out = GEB_index;
	GEB_index_out = (GEB_index - 1) & NB_SAMPLE_MASK;// GEB - 1 and Shift Phase TX +45 degree. So better sensitivity.
	// LCD info - line 2
	LCD_SetPos(0, 1);
	sprintf(str_val, "%d", GEB_index_out);
	LCD_String(str_val);
	LL_mDelay(1000);
	// debug
	printf("GEB Auto Setup: %lu\n", (uint32_t)GEB_index_out);
	// SET GEB
	Setup_GB_TIM1_CH1_CNT(0.00, 0, GEB_index_out, 1);
	
	return (uint32_t)GEB_index_out;
}
*/


void tone_button(uint8_t up_tone, uint8_t nb_tone, uint32_t len_ms)
{
	// Sound use TIM5 PWM Generation CH4 - PA3
	// TIM5 Prescaler = 84-1; 
	// Counter period = 1000-1;// (AutoReload Register - 32 bits value).
	// PWM mode 1  
	// Pulse = 500-1;// Pulse (32 bits value).
	
	uint32_t cnt_value;
	
	if (up_tone){
		//Audio_cnt = 99;// Enable 1 kHz		
		cnt_value = 1000;// 1000 Hz
	} else{
		//Audio_cnt = 199;// Enable 500 Hz
		cnt_value = 2000;// 500 Hz
	}	
	//if (len_ms > 500) {len_ms = 500;} 
	if (len_ms > 300) {len_ms = 300;} // for Morse code
	uint8_t i;
	for(i=0; i<nb_tone; i++){
		// Audio Enable
		Audio_EN = 1;// set flag
		// Writing values to registers
		TIM5->ARR = cnt_value;
		TIM5->CCR4 = (cnt_value >> 2);// divide by 4 - filling PWM 25 percent.
		//TIM5->CCR4 = (cnt_value >> 1);// divide by 2 - filling PWM 50 percent.
		// Enable capture/compare Channel CH4 
		LL_TIM_CC_EnableChannel(TIM5, LL_TIM_CHANNEL_CH4);
		// pause
		LL_mDelay(len_ms);
		// Disable capture/compare Channel CH4 
		LL_TIM_CC_DisableChannel(TIM5, LL_TIM_CHANNEL_CH4);
		Audio_EN = 0;// clr flag
		//PA3_CLR_BIT;
		// pause
		//LL_mDelay(len_ms);
		LL_mDelay(100);// Pause 100 ms.
	}
}

// TIM5_CH4
void Sound_beep(uint8_t tone_sel, uint32_t len_ms)
{
	
	// Sound use TIM5 - Output Compare CH4 - PA3
	// TIM5 Prescaler = 84-1; 
	// Counter period = 1000-1;// (AutoReload Register - 32 bits value).
	// Mode - Toggle on match 
	// Pulse = 250-1;// Pulse (32 bits value).
	
	uint32_t cnt_value;
	
	if (len_ms > 500) { len_ms = 500; }// max duration 
	
	if (tone_sel == 1) {
		cnt_value = 1000 - 1;// 1000 Hz
	}
	if (tone_sel == 2) {
		cnt_value = 2000 - 1;// 500 Hz
	}
	
	// Writing values to registers
	TIM5->ARR = cnt_value;
	TIM5->CCR4 = (cnt_value >> 2);// divide by 4 - filling PWM 25 percent.
	//TIM5->CCR4 = (cnt_value >> 1);// divide by 2 - filling PWM 50 percent.
	
	// Enable capture/compare Channel CH4 
	//LL_TIM_CC_EnableChannel(TIM5, LL_TIM_CHANNEL_CH4);
	//SET_BIT(TIM5->CCER, LL_TIM_CHANNEL_CH4);
	TIM5->CCER |= TIM_CCER_CC4E;
	
	// Enable timer counter
	//LL_TIM_EnableCounter(TIM5);// SET_BIT(TIM5->CR1, TIM_CR1_CEN);// TIM5->CR1 |= TIM_CR1_CEN;
	
	// pause - sound duration 
	LL_mDelay(len_ms);
	
	// Disable capture/compare Channel CH4 
	//LL_TIM_CC_DisableChannel(TIM5, LL_TIM_CHANNEL_CH4);
	//CLEAR_BIT(TIM5->CCER, LL_TIM_CHANNEL_CH4);
	TIM5->CCER &= ~TIM_CCER_CC4E;
	
	// Disable timer counter
	//LL_TIM_DisableCounter(TIM5);// CLEAR_BIT(TIM5->CR1, TIM_CR1_CEN);// TIM5->CR1 &= ~TIM_CR1_CEN;
}

void Fast_Setup(void)
{
	uint16_t Sense_max = 500;// = 250;
	// Barrier / Sense
	if (Button_UP()){ // UP Barrier+ / Low Sense
		if (Sense_value >= 10){// v1.0.1 // old value 40
			Sense_value += 5;
			//LL_mDelay(100);// 100 ms
		} else{
			Sense_value += 1;
			//LL_mDelay(100);// 100 ms
		}
		LL_mDelay(100);// 100 ms
		if (Sense_value > Sense_max) { Sense_value = 0; }
		LCD_Sense();
		if (Sense_value <= 10){ LL_mDelay(200); }// 200 ms
		// tones
		tone_button(1, 1, 100);// 1 kHz, nb 1, len 100 ms.
	}
	if (Button_DOWN()){ // Down Barrier- / Hight Sense
		if (Sense_value > 10){
			Sense_value -= 5;
			//LL_mDelay(100);// 100 ms
		} else{
			Sense_value -= 1;
			//LL_mDelay(100);// 100 ms
		}
		LL_mDelay(100);// 100 ms
		if (Sense_value > Sense_max) { Sense_value = Sense_max; }
		LCD_Sense();
		if (Sense_value <= 10){ LL_mDelay(200); }// 200 ms
		// tones
		tone_button(0, 1, 100);// 500 Hz, nb 1, len 100 ms.
	}
	// LEFT  - "Left (-) / Backlight"
	if (Button_LEFT()){ // Backlight ON / OFF
		/*
		SW_Backlight = 1 - SW_Backlight;// Switch - Disable \ Enable Backlight
		// SW_Backlight - flag and variable // volatile uint8_t SW_Backlight = 1;
		if (SW_Backlight == 0){
			BACKLIGHT_OFF;// #define BACKLIGHT_OFF (GPIOA->BSRR = GPIO_BSRR_BR1)
		} else {
			BACKLIGHT_ON;// #define BACKLIGHT_ON (GPIOA->BSRR = GPIO_BSRR_BS1)
		}
		*/
		/*
		// Switch - Disable \ Enable Signal buffering.
		SW_Signal_buff = 1 - SW_Signal_buff;
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		LCD_String("Signal Buffering");
		LCD_SetPos(0, 1);
		if (SW_Signal_buff == 0){
			LCD_String("OFF");
		} else {
			LCD_String(" ON");
		}
		*/
		// Switch Filter
		SW_Filter = (SW_Filter + 1 ) % 4;// modulo 4
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		LCD_String("Switch Filter");
		LCD_SetPos(0, 1);
		if (SW_Filter == 0){
			LCD_String(" 4 Hz");
		} else if (SW_Filter == 1){
			LCD_String(" 8 Hz");
		} else if (SW_Filter == 2){
			LCD_String("12 Hz");
		} else if (SW_Filter == 3){
			LCD_String("No Filter");
		}
		printf("\nSwitch Filter, case: %d\n", (int)SW_Filter);
		//
		//LL_mDelay(500);// 500 ms
		// tones
		tone_button(1, 1, 300);// 1 kHz, nb 1, len 300 ms.
		LL_mDelay(500);// 500 ms
	}// end if
	// RIGHT - "Right (+) / PinPointer" ???
	// RIGHT - DEBUG MODE 0 / MODE 1
	if (Button_RIGHT()){ // Backlight ON / OFF
		SW_Mode = 1 - SW_Mode;// Switch - MODE 0 / MODE 1
		if (SW_Mode == 0){
			LCD_Clear();
			LCD_String("SW Mode 0");
			LCD_SetPos(0, 1);
			LCD_String("RX bit offset: 0");//LCD_String("RX bit: 1");
			//RX_Sample_bit = 1; 
			RX_Sample_bit_offset = 0;
			// tones
			tone_button(0, 1, 300);// 500 Hz, nb 1, len 300 ms.
		} else {
			LCD_Clear();
			LCD_String("SW Mode 1");
			LCD_SetPos(0, 1);
			LCD_String("RX bit offset: 1");//LCD_String("RX bit: 2");
			//RX_Sample_bit = 2; 
			RX_Sample_bit_offset = 1;
			// tones
			tone_button(0, 2, 300);// 500 Hz, nb 2, 300 ms.
		}
		LL_mDelay(1000);// 1000 ms
	}	
	// OK    - "Enter / OK / Ground balance"	
	if (Button_OK()){
		
		// Ground balance calibration
		Calib_auto = 1 - Calib_auto;// Switch - Disable \ Enable auto calibration
		// LCD info
		LCD_Clear();
		//LCD_SetPos(0, 0);
		LCD_String("Auto Calibration");//LCD_String("Calibration Auto");
		LCD_SetPos(0, 1);
		if (Calib_auto){
			LCD_String(" ON");
			// tones - Morse code: •−− 
			tone_button(1, 1, 100);// 1 kHz, nb 1, 100 ms.
			tone_button(1, 2, 300);// 1 kHz, nb 2, 300 ms.
		} else {
			LCD_String("OFF");
			// tones - Morse code: −−− 
			//tone_button(1, 1, 100);// 1 kHz, nb 1, 100 ms.
			tone_button(1, 3, 300);// 1 kHz, nb 3, 300 ms.
		}
		//LL_mDelay(1000);// 1000 ms
		//LL_mDelay(2000);// 2000 ms
		LL_mDelay(1500);// 1500 ms
		// Calibration
		if (!Calib_auto){
			MD_StartCalibration();
			//Target_flag = 1;
		}
		Target_flag = 1;		
	}// end if Button_OK()
}


// Match Speed Filter
void Match_Speed_Filter(uint8_t sp)
{
	uint8_t Select_Speed = sp;
	printf("Filter Speed: %lu\n", (uint32_t)Select_Speed);// debug
	
	switch (Select_Speed) {
		// case 0 - 4 16 Hz - 64 Hz
		case 0:// 16 Hz
		T_H_CNT = T_16Hz;// T_H_CNT TIM2 Callback
		//Fd = (float)(16 * _N);// 16 * 6 = 96 Hz - Filter Sampling frequency
		//Freq_OVS = (uint16_t)(16 * _N);// #define _N 6
		break;
		
		case 1:// 24 Hz
		T_H_CNT = T_24Hz;
		//Fd = (float)(24 * _N);// 24 * 6 = 144 Hz 
		//Freq_OVS = (uint16_t)(24 * _N);
		break;
		
		case 2:// 32 Hz
		T_H_CNT = T_32Hz;
		//Fd = (float)(32 * _N);// 32 * 6 = 192 Hz 
		//Freq_OVS = (uint16_t)(32 * _N);
		break;
		
		case 3:// 48 Hz
		T_H_CNT = T_48Hz;
		//Fd = (float)(48 * _N);// 48 * 6 = 288 Hz 
		//Freq_OVS = (uint16_t)(48 * _N);
		break;
		
		case 4:// 64 Hz
		T_H_CNT = T_64Hz;
		//Fd = (float)(64 * _N);// 64 * 6 = 384 Hz 
		//Freq_OVS = (uint16_t)(64 * _N);
		break;
		
		default:// 32 Hz default
		T_H_CNT = T_32Hz;
		//Fd = (float)(32 * _N);// 32 * 6 = 192 Hz 
		//Freq_OVS = (uint16_t)(32 * _N);
		break;		
	}
	// Filter init
	//Filter_Init();
	//
	// Counter oversampling
	// TIM2_DIV 10 // 100000 = 10us.
	//OVS = 100000 / (uint16_t)Freq_OVS;// Frequency oversampling 32 Hz * _N
	OVS = 1000;// 100000 / 100 Frequency oversampling 100 Hz
	//
	//printf("OVS Cnt: %lu Fd: %ld Freq_OVS: %lu\n", (uint32_t)OVS, (int32_t)Fd, (uint32_t)Freq_OVS);// debug
	printf("OVS Cnt: %lu Freq_OVS: %lu\n", (uint32_t)OVS, (uint32_t)Freq_OVS);// debug
}


// Battery voltmeter
void Battery_Voltmeter(void)
{
	volatile float Ubat = 0.00f;
	volatile float Uref = 3.30f;// 3,3 V as Vdd
	volatile float ch2_izm = 0.0f;
	volatile float res_R1 = 22.00f;// 22 kOm
	volatile float res_R2 = 3.30f;// 3,3 kOm (R input ADC1 30 kOm ?)
	// Select Regular Channel 2
	LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_2);
	LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_2, LL_ADC_SAMPLINGTIME_3CYCLES);
	LL_mDelay(100);
	// Average ADC value
	for(uint8_t i = 0; i < 4; i++){
		ch2_izm += (float)ADC1_value;
		LL_mDelay(50);
	}
	ch2_izm = ch2_izm * 0.25;// divide by 4 - average ADC value
	// Conversion to Voltage
	Ubat = (ch2_izm * Uref) / 4096;
	// Voltage divider. Uout =  Uin * R2 / (R1 + R2) 
	float res_R1_R2 = res_R1 + res_R2;
	Ubat = Ubat * res_R1_R2 / res_R2;// Uin = (Uout * (R1 + R2)) / R2 
	// Select Regular Channel 6
	LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_6);
	LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_6, LL_ADC_SAMPLINGTIME_3CYCLES);
	LL_mDelay(100);
	printf("Battery: %.2f\n", Ubat);
	// LCD
	LCD_Clear();
	LCD_String("Battery Voltage");
	LCD_SetPos(0, 1);
	sprintf(str_02, " %.2f", Ubat);
	LCD_String(str_02);
	LL_mDelay(3000);// 3000 ms
}
// Battery voltmeter end code 

// KT0803L Init
void KT0803L_Init()
{
// Cannel number FM Transmitter - Read from EEPROM memory 
  i2c_init();
  KT0803L_Channel_tmp = EEPROM_Read16(EEPROM_ADDR_KT0803L_FM_CANNEL);// addr 0x0010
  KT0803L_Channel = KT0803L_Channel_tmp;// set channel
  printf("EEPROM - KT0803L Cannel: %lu\n", (uint32_t)KT0803L_Channel_tmp);
  if ((KT0803L_Channel_tmp > 2160) || (KT0803L_Channel_tmp < 1400)) { 
	KT0803L_Channel = 2160;// 108 MHz
	printf("Set Channel 2160 Cannel: %lu 108.00 MHz\n", (uint32_t)KT0803L_Channel);
  } 
  KT0803L_Frequency = KT0803L_ChannelToFrequency(KT0803L_Channel);
  KT0803L_i2c_init();
  KT0803L_SetFrequency(KT0803L_Frequency, PA_BIAS_enable);
  //LL_mDelay(100);// 100 ms
  // LCD - FM Transmitter
  LCD_Clear();// clr
  LCD_String(" FM Transmitter ");
  LCD_SetPos(0, 1);
  sprintf(str_02, " %.2f MHz", KT0803L_Frequency);
  LCD_String(str_02);
  //
  // FM Transmitter settings. 
  KT0803L_Set_Main_Tunings();
  //
  LL_mDelay(2000);// 2000 ms
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /* Peripheral interrupt init*/
  /* PVD_IRQn interrupt configuration */
  NVIC_SetPriority(PVD_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(PVD_IRQn);

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  
  // FPU settings
  ENABLE_FPU;// #define ENABLE_FPU SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));// set CP10 and CP11 Full Access 
  
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
  //
  // Config PVD - Programmable voltage detector
  // Power control register 
  PWR->CR |= PWR_CR_PLS_LEV5;//PWR->CR |= PWR_CR_PLS_2V7;
  //PWR->CR |= PWR_CR_PLS_LEV7;//PWR->CR |= PWR_CR_PLS_2V9;
  PWR->CR |= PWR_CR_PVDE;// Enable PVD
  // Permissions
  EXTI->IMR |= EXTI_IMR_MR16;// Interrupt mask register 
  // if < 2,9V
  EXTI->RTSR |= EXTI_RTSR_TR16;// Rising trigger selection register 
  // if > 2,9V
  //EXTI->FTSR |= EXTI_FTSR_TR16;// Falling trigger selection register 
  // Clr flag
  EXTI->PR = EXTI_PR_PR16;// Pending register 
  // end Config PVD
  
  // LCD init
  LCD_init();
  
  // Config DMA
  LL_DMA_ConfigTransfer(DMA2, // param  DMAx 
                        LL_DMA_STREAM_0, // param  Stream LL_DMA_STREAM_0
                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                        LL_DMA_MODE_CIRCULAR              |
                        LL_DMA_PERIPH_NOINCREMENT         |
                        LL_DMA_MEMORY_INCREMENT           |
                        LL_DMA_PDATAALIGN_HALFWORD        |
                        LL_DMA_MDATAALIGN_HALFWORD        |
                        LL_DMA_PRIORITY_HIGH              );  
  // Set DMA transfer addresses of source and destination 
  LL_DMA_ConfigAddresses(DMA2, // param  DMAx 
                        LL_DMA_STREAM_0, // param  Stream LL_DMA_STREAM_0
                        LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
                        (uint32_t)&ADC1_value,
                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  // Set length to be transmitted 
  LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, 1);
  // Enable Transfer complete interrupt
  LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);
  //LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_0);// Enable Transfer error interrupt 
  //LL_DMA_DisableIT_TE(DMA2, LL_DMA_STREAM_0);// Disable Transfer error interrupt 
  // Enable Stream 0 
  LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
  
  // clr flags 
  //LL_DMA_ClearFlag_TC0(DMA2); //TX Transfer complete clear
  //LL_DMA_ClearFlag_TE0(DMA2); //TX Transfer error clear
  //LL_DMA_ClearFlag_FE0(DMA2); //TX Fifo error clear
  //LL_DMA_ClearFlag_HT0(DMA2);
  //LL_DMA_ClearFlag_DME0(DMA2);
  
  // end Config DMA
  
  // debug printf
  //tmpreg = READ_BIT(DMA2->LISR, DMA_LISR_TCIF0);
  //printf("\nDMA2 DMA_LISR_TCIF0 flag = %lu\n\n", tmpreg);
  //LL_DMA_ClearFlag_TC0(DMA2);// Clear Stream 0 transfer complete flag.
  // end debug  
  
  //Config ADC1
  // Enable external trigger
  //LL_ADC_REG_StartConversionExtTrig(ADC1, LL_ADC_REG_TRIG_EXT_RISING);// !!! already configured 
  // Select external trigger
  //LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_EXT_TIM1_CH1);// !!! already configured 
  
  // ADC Enable
  LL_ADC_Enable(ADC1);
  LL_mDelay(100);
  
  // Calibration ADC1 - if Use STM32F103
  //LL_ADC_StartCalibration(ADC1);
  //while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0) {};// wait
  //LL_mDelay(10);
  
  Timer_Match();// Match data TIM1
  
  // Init timers
  // Timer TIM1 OC Mode
  LL_TIM_SetAutoReload(TIM1, (uint32_t)(TIM1_CNT_Period - 1));
  TIM1->CCR1 = (uint32_t)((TIM1_CNT_Period / 2) - 1);// 124 tck //LL_TIM_OC_SetCompareCH1(TIM1, (uint32_t)TIM1_CNT_CompareValue);
  //TIM1->CCR1 = 0;
  
  //LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_OC1REF);// !!! THE ERROR !!!
  //LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_UPDATE);// !!! OK and already configured 
  
  // Timer TIM1_CH1 - PA8. Output pin. Check Frequency "TX" multiple by 8.
  // Enable Output !!! 
  // SetBit MOE in BDTR register !!!
  LL_TIM_EnableAllOutputs(TIM1);// SET_BIT(TIM1->BDTR, TIM_BDTR_MOE);
  SET_BIT(TIM1->CCER, TIM_CCER_CC1E);// CH1 OUTPUT ENABLE !!! 
  // end Enable Output 
  
  /* Cигнал "TX" */
  // Timer TIM3 OC Mode
  LL_TIM_SetAutoReload(TIM3, (uint32_t)NB_SAMPLE_MASK);// TIM3->ARR = 8-1;
  //LL_TIM_OC_SetCompareCH3(TIM3, (uint32_t)NB_SAMPLE_MASK);
  //TIM3->CCR4 = (uint32_t)TIM3_GEB_value;// 4-1;
  WRITE_REG(TIM3->CCR4, TIM3_GEB_value);
  
  // TIM1 check printf
  tmpreg = TIM1->ARR;
  printf("\n TIM1->ARR = %lu \n", (uint32_t)tmpreg);
  tmpreg = TIM1->CCR1;
  printf("\n TIM1->CCR1 = %lu \n", (uint32_t)tmpreg);
  // TIM3 check printf
  tmpreg = TIM3->ARR;
  printf("\n TIM3->ARR = %lu \n", (uint32_t)tmpreg);
  tmpreg = TIM3->CCR4;
  printf("\n TIM3->CCR4 = %lu \n", (uint32_t)tmpreg);
  //
  
  // Timer TIM3_CH4
  LL_TIM_EnableAllOutputs(TIM3);// SET_BIT(TIM3->BDTR, TIM_BDTR_MOE);
  SET_BIT(TIM3->CCER, TIM_CCER_CC4E);// CH4 OUTPUT ENABLE !!!
  
  // TIMER 2 - Timer Base for Oversampling
  // Timer TIM2_CH1
  LL_TIM_SetAutoReload(TIM2, (uint32_t)(TIM2_DIV - 1));// TIM2->ARR = 10-1;
  LL_TIM_OC_SetCompareCH1(TIM2, (uint32_t)(TIM2_DIV / 2));// TIM2->CCR1 = (uint32_t)(TIM2_DIV / 2);
  // Disable pending flag
  if (LL_TIM_IsActiveFlag_UPDATE(TIM2)){// READ_BIT(TIM2->SR, TIM_SR_UIF);
	LL_TIM_ClearFlag_UPDATE(TIM2);
  }
  // debug info
  tmpreg = READ_BIT(TIM2->SR, TIM_SR_UIF);//LL_TIM_IsActiveFlag_UPDATE(TIM2);
  tmpreg1 = TIM2->PSC;
  tmpreg2 = TIM2->ARR;
  tmpreg3 = TIM2->CCR1;
  printf("TIM2 TIM_SR_UIF flag = %lu \nTIM3->PSC = %lu TIM2->ARR = %lu TIM2->CCR1 = %lu\n\n", tmpreg, tmpreg1, tmpreg2, tmpreg3);
  // end debug
  
  // TIM2 Enable Interrupt
  LL_TIM_EnableIT_UPDATE(TIM2);
  //LL_mDelay(10);
  
  // USART1 Enable Interrupt
  //LL_USART_EnableIT_RXNE(USART1);
  //LL_USART_EnableIT_ERROR(USART1);
  
  //while(1) {};// STOP
  
  // Start Timer 1, 2, 3, 5.
  LL_TIM_EnableCounter(TIM1);// SET_BIT(TIM1->CR1, TIM_CR1_CEN);
  LL_TIM_EnableCounter(TIM2);// SET_BIT(TIM2->CR1, TIM_CR1_CEN);
  LL_TIM_EnableCounter(TIM3);// SET_BIT(TIM3->CR1, TIM_CR1_CEN);
  LL_TIM_EnableCounter(TIM5);// SET_BIT(TIM5->CR1, TIM_CR1_CEN);
  // OK
  // Read U_ID
  //Read_Device_ID();
  //
  
  // BACKLIGHT
  BACKLIGHT_ON;
  LL_mDelay(1000);// START SCH PCB  
  
  // TEST EEPROM 24C16N
  /*
  // write
  printf("TEST EEPROM\n");
  volatile uint16_t pages = 128;// 24C16
  printf("Write %d pages\n", pages);
  volatile uint16_t a = 0;
  volatile uint16_t val, val2 = 0;
  for(a = 0; a < pages; a++){
	  LL_mDelay(200);// 200 ms
	  i2c_init();
	  uint16_t addr = a * 16;
	  val2 = val * 256;
	  EEPROM_Write16(addr, val2);//EEPROM_Write16(addr, val);
	  printf("%02lX ", (uint32_t)val2);//printf("%02lX ", (uint32_t)val);
	  val++;
  }
  LL_mDelay(200);// 200 ms
  // read
  printf("\nRead\n");
  for(a = 0; a < pages; a++){
	  LL_mDelay(200);// 200 ms
	  i2c_init();
	  uint16_t addr = a * 16;
	  val = EEPROM_Read16(addr);
	  printf("%02lX ", (uint32_t)val);
  }  
  printf("\nEND TEST EEPROM\n");
  */
  // END TEST EEPROM  
  
  // Update F_TX from EEPROM memory
  i2c_init();
  volatile uint16_t F_TX_R = EEPROM_Read16(EEPROM_ADDR_FREQ_TX);// addr 0x0020
  printf("EEPROM Read F_TX_R: %lu\n", (uint32_t)F_TX_R);
  if ( (F_TX_R > (F_TX_Min - 1)) && (F_TX_R < (F_TX_Max + 1)) ) {
	F_TX = F_TX_R;
	printf("Update F_TX: %lu from EEPROM\n", (uint32_t)F_TX); 
  }
  // Update GEB from EEPROM memory
  i2c_init();
  volatile uint16_t GEB_R = EEPROM_Read16(EEPROM_ADDR_GEB);// addr 0x0030
  if (GEB_R > (uint16_t)NB_SAMPLE_MASK) GEB_R = (uint16_t)NB_SAMPLE_MASK;
  TIM3_GEB_value = (uint32_t)GEB_R;
  WRITE_REG(TIM3->CCR4, TIM3_GEB_value);// update timer - write reg. TIM3->CCR4
  printf("Update TIM3_GEB_value: %lu from EEPROM\n", (uint32_t)TIM3_GEB_value); 
  // Update Filter Speed from EEPROM memory
  i2c_init();
  volatile uint16_t SPEED_R = EEPROM_Read16(EEPROM_ADDR_FILTER_SPEED);// addr 0x0040
  if (SPEED_R > 4) SPEED_R = 2;// default 2
  Speed_Ind = (uint8_t)SPEED_R;// update
  printf("Update Speed_filter: %lu from EEPROM\n", (uint32_t)Speed_Ind); 
  // !!! Update F_TX
  SetFrequencyTX(F_TX);
  //
  //printf("VERSION 1.0.2 Frequency Auto Search 15000 Hz - 31000 Hz.\n");
  //printf("VERSION 1.0.3 Frequency Auto Search 15000 Hz - 31000 Hz. \nF_TX_Output + 0 Hz and Calib_auto = 1.\n");
  //printf("VERSION 1.0.4 Frequency Auto Search 15000 Hz - 31000 Hz. \nComparator var 1.1 \n");
  //printf("VERSION 1.0.5 Fixed. Frequency 6000 Hz - 31000 Hz. \nComparator var 1.1 \n");
  //printf("VERSION 1.0.6 Z-1 buff. Frequency Auto Search 15000 Hz - 31000 Hz. \nComparator var 1 \n");
  //printf("VERSION 1.0.7 Fixed. Frequency 6000 Hz - 31000 Hz. \nComparator var 1 \n");
  //printf("VERSION 1.0.8 Filter 2Hz. Frequency 6000 Hz - 31000 Hz. \nComparator var 1 \n");
  //printf("VERSION 1.0.9 FIR High Pass Bessel 4Hz. Frequency 6000 Hz - 31000 Hz. \nComparator var 1 \n");
  //printf("VERSION 1.0.10 Switch Filter 4Hz, 8Hz, 12Hz. Frequency 6000 Hz - 31000 Hz. \nFIXED VERSION !!!\n");
  printf("VERSION 1.0.18 Fixed. Date 4.11.2024.\n");
  //
  // Call AutoSearch TIM1_CNT_Tmp and use ADC1
  if(AutoSearch_fl){
	tone_button(1, 1, 300);
	uint8_t flag_freq_set = 1;
	F_TX_AutoSearch(TIM1_CNT_Min, TIM1_CNT_Max, flag_freq_set);// v1.0.2 Freq Auto 
	tone_button(1, 3, 100);
  }
  
  // RDP - Write Read Out Protection BIT !!!
  //Flash_Write_RDP_Level1();
  
  //CMP_UID_0();// UID Check
  
  //LED1_OFF;
  // LOGO
  LogoVersion();
  
  // FM Transmitter On / Off - Read from EEPROM memory 
  i2c_init();
  KT0803L_ON_flag = EEPROM_Read16(EEPROM_ADDR_KT0803L_ON_OFF);// addr 0x00B0
  KT0803L_ON_flag &= 0x01;// by mask 1 
  // Set bit and Reset bit PB2. PB2 connected to pin SW1 KT0803L. 
  if (KT0803L_ON_flag == 1) {
	// Enable chip KT0803L 
	KT0803L_ON;// GPIOB->BSRR = GPIO_BSRR_BS2;
	LL_mDelay(500);// 500 ms
	// FM Transmitter Set Frequency and settings. 
	KT0803L_Init();
	//
  } else {
	// Disable chip KT0803L 
	KT0803L_OFF;// GPIOB->BSRR = GPIO_BSRR_BR2;
	// LCD - FM Transmitter Off 
	LCD_Clear();// clr
	LCD_String("FM Transmitter");
	LCD_SetPos(0, 1);
	LCD_String("OFF");
	LL_mDelay(2000);// 2000 ms
  }
  //
  
  // Frequency !!!
  LCD_Clear();// clr
  //LCD_SetPos(0, 0);
  LCD_String("TX Frequency");
  LCD_SetPos(0, 1);
  sprintf(str_02, "%lu Hz", (uint32_t)F_TX);
  LCD_String(str_02);
  LL_mDelay(2000);// 2000 ms
  //
  // Battery voltmeter
  Battery_Voltmeter();
  
  // Read SENSE from EEPROM 
  i2c_init();
  Sense_value = EEPROM_Read16(EEPROM_ADDR_SENSE);// addr 0x0050
  if ((Sense_value == 0) || (Sense_value == 0xFFFF)) Sense_value = 50; 
  //
  // Read Switch Buffering Mode from EEPROM 
  i2c_init();
  SW_Signal_buff = (uint8_t)EEPROM_Read16(EEPROM_ADDR_SW_BUFFER_MODE);// addr 0x0060
  //if (SW_Signal_buff > 1) SW_Signal_buff = 1;// default 1 
  SW_Signal_buff &= 0x01;// by mask 1 
  //
  // Read RX Sample bit offset from EEPROM 
  i2c_init();
  RX_Sample_bit_offset = (uint8_t)EEPROM_Read16(EEPROM_ADDR_RX_MODE);// addr 0x0080
  RX_Sample_bit_offset &= 0x01;// by mask 1 
  //
  // Read Mask Ferrum flag from EEPROM 
  i2c_init();
  Mask_Ferrum_flag = (uint8_t)EEPROM_Read16(EEPROM_ADDR_FERRUM_MASK);// addr 0x0090
  Mask_Ferrum_flag &= 0x01;// by mask 1 
  //
  
  // UID Protection !!!
  //if(UID_0_OK != 1){
	//LCD_Clear();
	//LCD_String("CPU ID ERROR");
	//while(1) {
	//	//LED1_INV; 
	//	LL_mDelay(250); 
	//} 
  //}
  // end UID Protection !!!
  
  //CMP_UID_1();// UID Check
  
  // Ground Balance - Tune BG
  //TIM3_GEB_value = 7;// value 0 - 7 Default = 7. // = (uint32_t)NB_SAMPLE_MASK;
  //Setup_GB_TIM1_CH1_CNT(BG_Tune_Ferr, 0, TIM3_GEB_value, 1);
  
  // bad //GEB Auto - bad function !!!
  // bad //TIM3_GEB_value_ = GEB_AutoSetup();// Get GEB
  
  // BG Tune Ferrite
  //Setup_GB_TIM1_CH1_CNT(BG_Tune_Ferr, 1, 0, 0);
  
  // end Ground Balance
  
  // Select counter
  //T_H_CNT = T_32Hz;//T_28Hz;//T_24Hz;
  Match_Speed_Filter(Speed_Ind);// 32 Hz //T_H_CNT = T_32Hz;
  T_L_CNT = T_5Hz;//T_4Hz;//T_2Hz;// Auto callib
  
  // Counter oversampling
  //OVS = 100000 / (uint16_t)Fd;// 1Mhz / 100000; TIM2_DIV = 10 us.
  //OVS = 100000 / (uint16_t)Freq_OVS;// Frequency oversampling 32 Hz * _N
  //printf("OVS Cnt: %lu Fd: %ld Freq_OVS: %lu\n", (uint32_t)OVS, (int32_t)Fd, (uint32_t)Freq_OVS);// debug
  // Match filter
  //Filter_Init();// str 1324 in Match_Speed_Filter(0x2);
  //
  
  //CMP_UID_2();// UID Check
  
  // Pause 800 ms
  LCD_Clear();
  //LCD_SetPos(0, 0);
  LCD_String("Good luck to you");//LCD_String("Filter init");
  LCD_SetPos(0, 1);
  uint8_t j;
  for(j=0; j<16; j++) {
	LCD_String("!");//LCD_String("*");
	LL_mDelay(50);//LL_mDelay(100);
  }
  // end pause
  
  //CMP_UID_3();// UID Check
  
  //DWT_enable();// DWT Counter !!!
        
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // While Main Program
	
	// Get Calibration Samples
	if (Calib_flag && (sw_default == 1)) {
	//if (Calib_flag && (sw_default == 1) && (UID_2_OK == 1)) { error: - check long time // Check OK UID 2
		LCD_Clear();
		//LCD_SetPos(0, 0);
		sprintf(str_01, "%s", str_wait);
		LCD_String(str_01);//LCD_String("Please wait...");
		LL_mDelay(1500);// !!! Pause 1500 ms !!! - Time to average data !!! 
		MD_StartCalibration();
	}
	
	// Main menu
	MenuProcessProg();// Polling Buttons
	// Fast setup
	if (sw_default == 1) {// if set flag default menu
		Fast_Setup();// Sense Barrier
	}
	
	// AUDIO
	uint8_t tone_select;
	//printf("Target_flag: %lu\n", (uint32_t)Target_flag);
	if (Target_flag){
		GetVDI_Value();
		if (VDI_value >= 0){// Audio tone
			//Audio_cnt = 99;// Enable 1 kHz
			tone_select = 1;// 1 kHz
		} else {
			//Audio_cnt = 199;// Enable 500 Hz
			tone_select = 2;// 500 Hz
		}
		// The Fast?
		//LL_mDelay(20);// 1/50Hz ?
		Target_flag = 0;// clr flag !!! 
		// end The Fast?
		// Audio Enable
		Audio_EN = 1;
		// Mask Ferrum - Disable Audio
		if ((Mask_Ferrum_flag == 1) && (VDI_value < VDI_Ferrum_max)) { Audio_EN = 0; }
		//
		//LED1_ON;
		// Len Audio ms
		volatile int32_t Len_Audio_val = Ph_diff;
		if (Len_Audio_val < 0) { Len_Audio_val = ~Len_Audio_val; }
		if (Len_Audio_val > 400) { Len_Audio_val = 400; } // Maximum 400 ms
		uint32_t tone_len_ms = 100 + (uint32_t)(Len_Audio_val >> 2);// Minimum 100 ms + Maximum value divide by 4
		//LL_mDelay(tone_len_ms);// f103
		//Sound_beep(tone_select, tone_len_ms);// SOUND 
		if (Audio_EN == 1) { Sound_beep(tone_select, tone_len_ms); } // SOUND 
		// pause ?
		// v1.0.1
		//uint32_t pause_len_ms = (tone_len_ms >> 3);// divide by 8
		//LL_mDelay(pause_len_ms);// v1.0.1
		//LL_mDelay(100);// ?
		//Target_flag = 0;// clr flag
		Audio_EN = 0;// Disable
		//LED1_OFF;
		//if (Audio_EN == 0){ PA3_CLR_BIT; }
		//LL_mDelay(100);// debug
	//}
	if (sw_default == 1) {// if set flag default menu
	//if ((sw_default == 1) && (UID_3_OK == 1)) {// if set flag default menu and Check OK UID 3
		//MenuVDI((int16_t)VDI_value, (int32_t)Amp_diff);// VDI to LCD1602
		MenuVDI((int16_t)VDI_value, (int32_t)Ph_diff);// ok
	}
	
	// Check LL_mDelay()
	/*
	while (1)
	{
		LED1_ON;
		LL_mDelay(1000);//LL_mDelay(1000);
		LED1_OFF;
		LL_mDelay(1000);
	}
	*/
	
	// debug
	if (Uart_LogLevel == 1){
	printf("\nVDI %ld \n", (int32_t)VDI_value);
	printf("--->    Target_value %ld Filter_Out %ld\n", (int32_t)Target_value, (int32_t)Filter_Out);	
	// diff
	printf("\n-> Ph_diff %ld Amp_diff  %ld\n", (int32_t)Ph_diff, (int32_t)Amp_diff);
	
	// debug
	//uint8_t i;
	//for(i=0;i<NB_SAMPLE;i++){		
		
		//while (!(Sample_index == i)) {};
		//printf("Sample_index %lu Sample_value  %lu\n", (uint32_t)Sample_index, (uint32_t)Sample_value[Sample_index]);
		//printf("Sample_index %lu Sample_value  %ld\n", (uint32_t)Sample_index, (int32_t)Sample_value[Sample_index]);
		//printf("Sample_index %lu Average_value %ld\n", (uint32_t)Sample_index, (int32_t)Average_value[Sample_index]);
		
	//}
	// debug info
	uint8_t i;
	for(i=0;i<NB_SAMPLE_DIV2;i++){//for(i=0;i<NB_SAMPLE;i++){
		while (!(Sample_index == i)) {};//while (!(Sample_index == 1)) {};
		printf("Sample_index %lu Average_value %ld      Amp_Average_value %lu\n", (uint32_t)Sample_index, (int32_t)Average_value[Sample_index], (uint32_t)Amp_Average_value[Sample_index]); 
	}
	//
	printf("---> Filter_In %ld Filter_Out %ld\n", (int32_t)Filter_In, (int32_t)Filter_Out);
	//
	//printf("---> TIM1->CNT: %lu\n", tmpreg1);
	// debug
	//printf("TIM1 Sampl ind: %lu\n", tmpreg3);
	//printf("---> TIM2->CNT: %lu\n", tmpreg2);
	//printf("\n\n---> START DEBUG INFO <---\n");
	//
	}// end Uart_LogLevel 1
	
	}// end if Target_flag
	
	//LL_mDelay(100);
	
	// UID Protection !!!
	//if(UID_1_OK != 1){
		//Target_flag = 1;// set flag
		//LL_mDelay(1000);
		//LED1_INV;
	//}
	// end UID Protection !!!
	
	// End code
	
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_25, 168, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_Init1msTick(84000000);
  LL_SetSystemCoreClock(84000000);
  LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**ADC1 GPIO Configuration
  PA2   ------> ADC1_IN2
  PA6   ------> ADC1_IN6
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* ADC1 DMA Init */

  /* ADC1 Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_0, LL_DMA_CHANNEL_0);

  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_0, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_0, LL_DMA_PRIORITY_HIGH);

  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MODE_CIRCULAR);

  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_HALFWORD);

  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_HALFWORD);

  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_0);

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  LL_ADC_Init(ADC1, &ADC_InitStruct);
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM1_CH1;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_UNLIMITED;
  LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
  LL_ADC_REG_SetFlagEndOfConversion(ADC1, LL_ADC_REG_FLAG_EOC_UNITARY_CONV);
  ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);
  LL_ADC_REG_StartConversionExtTrig(ADC1, LL_ADC_REG_TRIG_EXT_RISING);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_6);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_6, LL_ADC_SAMPLINGTIME_3CYCLES);
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
  LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 999;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM1, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM1);
  LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_TOGGLE;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 499;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;
  LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_UPDATE);
  LL_TIM_DisableMasterSlaveMode(TIM1);
  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
  TIM_BDTRInitStruct.OSSRState = LL_TIM_OSSR_DISABLE;
  TIM_BDTRInitStruct.OSSIState = LL_TIM_OSSI_DISABLE;
  TIM_BDTRInitStruct.LockLevel = LL_TIM_LOCKLEVEL_OFF;
  TIM_BDTRInitStruct.DeadTime = 0;
  TIM_BDTRInitStruct.BreakState = LL_TIM_BREAK_DISABLE;
  TIM_BDTRInitStruct.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;
  TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;
  LL_TIM_BDTR_Init(TIM1, &TIM_BDTRInitStruct);
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**TIM1 GPIO Configuration
  PA8   ------> TIM1_CH1
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  /* TIM2 interrupt Init */
  NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),7, 0));
  NVIC_EnableIRQ(TIM2_IRQn);

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  TIM_InitStruct.Prescaler = 83;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 499;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM2);
  LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_TOGGLE;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 124;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM2, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_UPDATE);
  LL_TIM_DisableMasterSlaveMode(TIM2);
  LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH1);
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 7;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM3);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_TOGGLE;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 7;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH4, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH4);
  LL_TIM_SetTriggerInput(TIM3, LL_TIM_TS_ITR0);
  LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_EXT_MODE1);
  LL_TIM_DisableIT_TRIG(TIM3);
  LL_TIM_DisableDMAReq_TRIG(TIM3);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_UPDATE);
  LL_TIM_DisableMasterSlaveMode(TIM3);
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH4);
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**TIM3 GPIO Configuration
  PB1   ------> TIM3_CH4
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  TIM_InitStruct.Prescaler = 83;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 999;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM5, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM5);
  LL_TIM_SetClockSource(TIM5, LL_TIM_CLOCKSOURCE_INTERNAL);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_TOGGLE;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 249;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM5, LL_TIM_CHANNEL_CH4, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM5, LL_TIM_CHANNEL_CH4);
  LL_TIM_SetTriggerOutput(TIM5, LL_TIM_TRGO_UPDATE);
  LL_TIM_DisableMasterSlaveMode(TIM5);
  LL_TIM_OC_EnablePreload(TIM5, LL_TIM_CHANNEL_CH4);
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**TIM5 GPIO Configuration
  PA3   ------> TIM5_CH4
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9|LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* Init with LL driver */
  /* DMA controller clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  NVIC_SetPriority(DMA2_Stream0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

  /**/
  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13|LL_GPIO_PIN_14|LL_GPIO_PIN_15);

  /**/
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1|LL_GPIO_PIN_4|LL_GPIO_PIN_5|LL_GPIO_PIN_15);

  /**/
  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_2|LL_GPIO_PIN_12|LL_GPIO_PIN_13|LL_GPIO_PIN_14
                          |LL_GPIO_PIN_15|LL_GPIO_PIN_3);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_13|LL_GPIO_PIN_14|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1|LL_GPIO_PIN_4|LL_GPIO_PIN_5|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_12|LL_GPIO_PIN_13|LL_GPIO_PIN_14
                          |LL_GPIO_PIN_15|LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4|LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7
                          |LL_GPIO_PIN_8|LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void ADC_DMA_TransferComplete_Callback()
{
	//printf("ADC_DMA_TransferComplete_Callback()\n");// debug
	//PB8_SET_BIT;// debug
	// We use 4 samples out of 8 samples.
	if (Sample_index < (uint8_t)NB_SAMPLE_DIV2){// if < 4
		Sample_value[Sample_index & (uint8_t)NB_SAMPLE_DIV2_MASK] += ADC1_value;// Sample_index & 3
		// Amplitude
		//if (Samples_cnt < (uint8_t)NB_AVERAGE_DIV4){// if < 8
			//Amp_Sample_value[Sample_index] += ADC1_value;
		//}
		Amp_Sample_value[Sample_index] = (uint32_t)ADC1_value;// copy value
	}
	else{
		Sample_value[Sample_index & (uint8_t)NB_SAMPLE_DIV2_MASK] -= ADC1_value;// Sample_index & 3
		//Amp_Sample_value[Sample_index] += ADC1_value; Samples 4-7 not use.		
	}	
	Sample_index++;
	if (Sample_index > (uint8_t)NB_SAMPLE_MASK){// if > 7
		Sample_index = 0;
		Samples_cnt++;
		// debug
		//tmpreg1 = TIM1->CNT;// debug
	}
	if (Samples_cnt > (uint8_t)NB_AVERAGE_MASK){// if > 15
		Samples_cnt = 0;
		//memcpy((void*)Average_value, (void*)Sample_value, sizeof(Average_value));
		Average_value[0] = Sample_value[0];
		Average_value[1] = Sample_value[1];
		Average_value[2] = Sample_value[2];
		Average_value[3] = Sample_value[3];
		// Amplitude
		Amp_Average_value[0] = Amp_Sample_value[0];
		Amp_Average_value[1] = Amp_Sample_value[1];
		Amp_Average_value[2] = Amp_Sample_value[2];
		Amp_Average_value[3] = Amp_Sample_value[3];
		// Cleaning Samples
		//memset((void*)Sample_value, 0, sizeof(Sample_value));
		Sample_value[0] = 0;
		Sample_value[1] = 0;
		Sample_value[2] = 0;
		Sample_value[3] = 0;
		// Amplitude Cleaning Samples
		// If copying, then data cleaning is not required.
		//Amp_Sample_value[0] = 0;
		//Amp_Sample_value[1] = 0;
		//Amp_Sample_value[2] = 0;
		//Amp_Sample_value[3] = 0;
		// Set flag!
		Sample_Ready = 1;
		//
		// debug
		//tmpreg2 = TIM3->CNT;// debug
		//
		// TIMER 3 Synchronization Counter !!!
		// Used !!!
		TIM3->CNT = (uint16_t)NB_SAMPLE_MASK;// = 7
		// end
		// debug
		//tmpreg3 = (uint32_t)Sample_index;
	}	
	//PB8_CLR_BIT;// debug	
}

// PVD
void PVD_Callback(void)
{
	printf("VDD Less 2,7V !!!  \n");
	//printf("VDD Less 2,9V !!!  \n");
	NVIC_SystemReset();// CPU Reset	
}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  printf("\nError Handler\n");
	  //LL_mDelay(1000);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
