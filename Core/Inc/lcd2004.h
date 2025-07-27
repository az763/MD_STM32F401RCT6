/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lcd2004.h
  * @brief          : Header for lcd2004.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LCD2004_H_
#define LCD2004_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f401xc.h"


/* USER CODE BEGIN Private defines */
// CMSIS
#define LCD_D4_SET()   (GPIOB->BSRR = GPIO_BSRR_BS15)// PB15
#define LCD_D4_RESET() (GPIOB->BSRR = GPIO_BSRR_BR15)

#define LCD_D5_SET()   (GPIOB->BSRR = GPIO_BSRR_BS14)// PB14
#define LCD_D5_RESET() (GPIOB->BSRR = GPIO_BSRR_BR14)

#define LCD_D6_SET()   (GPIOB->BSRR = GPIO_BSRR_BS13)// PB13
#define LCD_D6_RESET() (GPIOB->BSRR = GPIO_BSRR_BR13)

#define LCD_D7_SET()   (GPIOB->BSRR = GPIO_BSRR_BS12)// PB12
#define LCD_D7_RESET() (GPIOB->BSRR = GPIO_BSRR_BR12)

#define e1             (GPIOA->BSRR = GPIO_BSRR_BS15)// PA15
#define e0             (GPIOA->BSRR = GPIO_BSRR_BR15)

#define rs1            (GPIOB->BSRR = GPIO_BSRR_BS3)// PB3
#define rs0            (GPIOB->BSRR = GPIO_BSRR_BR3)

#define LCD_DISPLAYCONTROL 0x08
#define LCD_DISPLAYON 0x04
//#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
//#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
//#define LCD_BLINKOFF 0x00

//uint8_t _displaycontrol = 0;

/* USER CODE END Private defines */


/* Exported functions prototypes ---------------------------------------------*/
void LCD_init(void);
void LCD_Clear(void);
void LCD_Home(void);
void LCD_SendChar(char ch);
void LCD_String(char* st);
void LCD_SetPos(uint8_t x, uint8_t y);
//
void delay(void);
//void mDelay(uint16_t val);
//void LCD_WriteData(uint8_t dt);
void LCD_Command(uint8_t dt);
//void LCD_Data(uint8_t dt);
void LCD_Cursor(void);
void LCD_noCursor(void);
void LCD_Blink(void);
void LCD_noBlink(void);


#ifdef __cplusplus
}
#endif

#endif /* LCD2004_H_ */
