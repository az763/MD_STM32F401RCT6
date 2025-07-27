#ifndef __FIR_LOWPASS_L6_H
#define __FIR_LOWPASS_L6_H

#ifdef __cplusplus
extern "C" {
#endif
//start code https://habr.com/ru/post/128140/ err
#include "main.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
/**
  * FIR LowPass Filter by alek76
  * file fir_lowpass.h
  * date 13.12.2022
*/

#define _N 6//4// 6 Fixed. #define NB_OVERSAMPLE (_N)
//const uint8_t N = _N - 1;// Длина фильтра = Количество элементов фильтра. 15 - нечётное.
const uint8_t N = _N;// Длина фильтра = Количество элементов фильтра. 4 - чётное.
//const uint8_t P = N-1;// Порядок фильтра = Количество линий задержки. 14 - чётное.
volatile float H_id[_N] = {0.0};// Идеальная импульсная характеристика
volatile float W[_N] = {0.0};// Весовая функция - Окно
volatile float H[_N] = {0.0};// Импульсная характеристика фильтра Float-point
volatile int32_t H_coeff[_N] = {0};// Импульсная характеристика фильтра Fixed-point = H * 2^14
#define FIXED_POINT_FRACTIONAL_BITS 14//14// 2^(14) = 16384
//volatile int32_t Y = 0;// Output value
//volatile int32_t pX[_N] = {0};// Буфер задержки

//volatile float Fd = 240.0;// 240 Hz Частота дискретизации входных данных 8*15*2 = 240
volatile float Fd = 192.0;//= 120.0;// = 80.0;// = 160.0;// 160 Hz Частота дискретизации входных данных 5*4*2 = 40 x 2

void Filter_Init()
{
//float Fd = 240.0;// 128.0;// 128 Hz Частота дискретизации входных данных 
float Fs = 5.0;//10.0;// = 8.0;// = 4.5; // Частота полосы пропускания 
float Fc = Fs / Fd;// Нормированная частота среза. Например равная 0,25 

// Basic Formula: H(n)= [2*Fc*sin(2*pi*Fc*n)]/(2*pi*Fc*n)
// ========================================
// Для НЕчётной длины, где N = 15
// H(0)=2*Fc; n=0;
// H(n)=[sin(2*pi*Fc*n)]/(pi*n);
// ========================================
// ========================================
// Для Чётной длины, где N = 16
// H(n)=[sin(2*pi*(n-0.5)*Fc)]/((n-0.5)*pi);
// ========================================

//Расчет импульсной характеристики фильтра
// N = 7, N = 15.

for (uint8_t i=0;i<N;i++){
	/*
	if (i == 0){	
		H_id[i] = 2 * Fc;
	} else {
		H_id[i] = sinl(2*M_PI*Fc*i )/(M_PI*i);		
	}
	*/
	H_id[i] = sinl(2*M_PI*(i-0.5)*Fc) / ((i-0.5)*M_PI);// N = 8, N = 16.
	
	// Весовая функция 0 <= i <= N - 1
	W[i] = 0.42 - 0.5 * cosl((2*M_PI*i) / ( N-1)) + 0.08 * cosl((4*M_PI*i) / ( N-1));// Окно Блэкмана (Blackman). 0 <= i <= N - 1
	//W[i] = 0.5 - 0.5 * cosl((2*M_PI*i) / ( N-1));// Окно Хэнна (Hanning). 0 <= i <= N - 1
	//W[i] = 0.54 - 0.46 * cosl((2*M_PI*i) / ( N-1));// Окно Хемминга (Hamming). 0 <= i <= N - 1	
	// Оконное сглаживание
	H[i] = H_id[i] * W[i];
	// debug
	//printf("index %lu  H_id: %ld \n", (uint32_t)i, (int32_t)(H_id[i] * (1 << FIXED_POINT_FRACTIONAL_BITS)));// x 2^14
	//printf("index %lu     W: %ld \n", (uint32_t)i, (int32_t)(W[i] * (1 << FIXED_POINT_FRACTIONAL_BITS)));// debug
	//printf("index %lu     H: %ld \n", (uint32_t)i, (int32_t)(H[i] * (1 << FIXED_POINT_FRACTIONAL_BITS)));// debug	
}

// Нормирование коэффициентов
volatile float SUM = 0;
for (uint8_t i=0; i<N; i++) { SUM += H[i]; }
//printf("SUM = 1x1000: %ld \n", (int32_t)(SUM * 1000));// debug
for (uint8_t i=0; i<N; i++) {
	H[i] /= SUM; // The sum of all coefficients H[i] is equal to one..
	// float-to-fixed point conversion
	//H_coeff[i] = (int32_t)(H[i] * (1 << FIXED_POINT_FRACTIONAL_BITS));// Mult 2^14 and conversion 
	// Round uses 434 bites in HEX
	H_coeff[i] = (int32_t)(round(H[i] * (1 << FIXED_POINT_FRACTIONAL_BITS)));// Mult 2^14 Round and conversion 
	//printf(" norm H_coeff: %ld \n", H_coeff[i]);// Сумма всех коэффициентов = 16384 = 2^FIXED_POINT_FRACTIONAL_BITS 	
}
}


/*
// Fir lowpass filter
#define NB_ZEROS _N// 16

int32_t FIR_lowpass_PH(volatile int32_t Input_value)
{
	static int32_t Xval[NB_ZEROS + 1];// NB 17
	static int32_t Output_value = 0;
	volatile uint8_t i;
	// Z-1
	for (i = 0; i < NB_ZEROS; i++){
		Xval[i] = Xval[i+1];// Xval[0] = Xval[1], ..., Xval[15] = Xval[16].
	}
	// Input
	Xval[NB_ZEROS] = Input_value;
	// Aply filter
	for (i = 0; i < NB_ZEROS; i++){
		Output_value =+ Xval[i] * H_coeff[i];
	}
	
	return (int32_t)(Output_value >> FIXED_POINT_FRACTIONAL_BITS);// Divide by 2^(14)
}
*/

//void FIR_Clr()
//{
	// Set 1 - Не умножать на ноль.
	//memset((void*)pX, 1, sizeof(pX));
//}

//start code https://www.cyberforum.ru/cpp-beginners/thread2425572.html
// The code is correct ???
/*
void FIR_lowpass_PH(int32_t B0, int32_t* pX)
{
	// Линия задержки не нужна. Перемножить входные данные на коэффициенты, 
	// где индекс следует в обратном порядке N-i.
	
	//volatile uint8_t Z = 0;
	//for (uint8_t i = N - 1; i != 0; i--){// P = N-1 Filter order = number of delay lines	
		//Z = i - 1;
		//pX[i] = pX[Z];// shift pX data
		// if N=8 P=7
		// pX7 = pX6
		// pX6 = pX5
		// pX5 = pX4
		// pX4 = pX3
		// pX3 = pX2
		// pX2 = pX1
		// pX1 = pX0		
	//}
	
	// Input data bit0
	// pX[0] = B0;// Этот бит так-же не нужен.
	// clr
	Y = 0;
	// Apply Filtration
	for (uint8_t i=0; i<N; i++){
		Y += H_coeff[N-i] * pX[i];//Y += H_coeff[i] * pX[i];
	}
	FIR_Clr();
	//printf("N = %lu FIR_lowpass_PH Y: %ld \n", (uint32_t)N, (int32_t)Y);// debug
	//return Y;
} 
*/
// end code  
#ifdef __cplusplus
}
#endif

#endif /* FIR_LOWPASS_H */
