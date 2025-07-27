/*
 * menu.h
 *
 *  Created on: Jan 6, 2023
 *      Author: as
 */

#ifndef INC_LCD1602_MENU_H_
#define INC_LCD1602_MENU_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "button.h"
#include "lcd2004.h"


/* USER CODE BEGIN Private defines */
#define MAIN_NENU_NB 9//8
#define MAIN_NENU_STEP 10
//#define MAIN_NENU_STEP_X2 (MAIN_NENU_STEP * 2)// 20
#define MAIN_NENU_MAX (MAIN_NENU_NB * MAIN_NENU_STEP)// 90 
#define SUB_MENU_NB 3
#define SUB_MENU_STEP 1
#define SUB_MENU_PROG 10

// GEB
#define NB_GEB 7 // NB_SAMPLE_MASK

/* Exported functions prototypes ---------------------------------------------*/
void SetFrequencyTX(uint16_t TX_freq);
void Setup_GB_TIM1_CH1_CNT(float gb, uint8_t gb_flag, uint32_t GEB_CNT_value, uint8_t GEB_flag);
void SetGEB(void);
void Get_AMP_mv(float bin0, float bin1, float bin2, float bin3);
void PollingButtons();
void MenuProcess();
void MenuProcessProg();
void MenuVDI(int16_t vdi_value, int32_t amp_value);
void SetFrequency();
void FM_SetFrequency();// FM Transmitter KT0803L 
void KT0803L_OnOff();
void KT0803L_Init();// in main.c
void Search_Freq(void);
void LCD_Sense();
void SetSense();
void SetBufferingMode();
void SetRXbit();
void SetMaskFerrum();
void Match_Speed_Filter(uint8_t sp);// in main.c


/* USER CODE END Private defines */


#ifdef __cplusplus
}
#endif

#endif /* INC_LCD1602_MENU_H_ */
