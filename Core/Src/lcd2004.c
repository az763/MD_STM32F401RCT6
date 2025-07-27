/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lcd2004.c
  * @brief          : lcd2004 program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lcd2004.h"
#include "stm32f4xx_ll_utils.h"// LL_mDelay()
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//------------------------------------------------
uint8_t _displaycontrol = 0;

void delay(void)// 59 us. 50 us = 3600 tick, 72 * 50 us = 3600 tick 
{
	volatile uint16_t i;// use volatile !!!
	//for(i=0;i<1000;i++)// 21020 tick / 72 = 292 us
	//for(i=0;i<171;i++)// 3617 tick / 72 = 50 us
	for(i=0;i<200;i++)// 4226 tick / 72 = 59 us	
	{
		
	}
}

/*
void mDelay(uint16_t val)// value ms
{
	volatile uint16_t nb_ms = val;
	volatile uint16_t j;
	volatile uint32_t i;
	for(j=0;j<nb_ms;j++){
		for(i=0;i<72000;i++)// 1 ms
		{
			
		}
	}
}
*/

//------------------------------------------------
void LCD_WriteData(uint8_t dt)
{
	if(((dt>>3)&0x01)==1) {LCD_D7_SET();} else {LCD_D7_RESET();}
	if(((dt>>2)&0x01)==1) {LCD_D6_SET();} else {LCD_D6_RESET();}
	if(((dt>>1)&0x01)==1) {LCD_D5_SET();} else {LCD_D5_RESET();}
	if((dt&0x01)==1) {LCD_D4_SET();} else {LCD_D4_RESET();}	
}
//------------------------------------------------
void LCD_Command(uint8_t dt)
{
	rs0;
	LCD_WriteData(dt>>4);
	e1;
	delay();
	e0;
	LCD_WriteData(dt);
	e1;
	delay();
	e0;
}
//------------------------------------------------
void LCD_Data(uint8_t dt)
{
	rs1;
	LCD_WriteData(dt>>4);
	e1;
	delay();
	e0;
	LCD_WriteData(dt);
	e1;
	delay();
	e0;
}
//------------------------------------------------
void LCD_Clear(void)
{
	LCD_Command(0x01);
	LL_mDelay(2);
}
//------------------------------------------------
void LCD_Home(void)
{
	LCD_Command(0x02);// set cursor position to zero
	LL_mDelay(2);
}
//------------------------------------------------
void LCD_SendChar(char ch)
{
	LCD_Data((uint8_t)ch);
	delay();
}
//------------------------------------------------
void LCD_String(char* st)
{
	uint8_t i=0;
	while(st[i]!=0)
	{
		LCD_Data(st[i]);
		delay();
		i++;
	}
}
//------------------------------------------------
void LCD_SetPos(uint8_t x, uint8_t y)
{
	switch(y)
	{
		case 0:
			LCD_Command(x|0x80);
			LL_mDelay(1);
			break;
		case 1:
			LCD_Command((0x40+x)|0x80);
			LL_mDelay(1);
			break;
		//case 2:// LCD20x4
			//LCD_Command((0x14+x)|0x80);
			//LL_mDelay(1);
			//break;
		//case 3:// LCD20x4
			//LCD_Command((0x54+x)|0x80);
			//LL_mDelay(1);
			//break;
	}
}
//------------------------------------------------
//#define LCD_DISPLAYCONTROL 0x08
//#define LCD_CURSORON 0x02
//#define LCD_BLINKON 0x01
void LCD_Cursor(void)
{
	_displaycontrol |= LCD_CURSORON;
	//LCD_Command(0x08 | 0x04 | 0x02);
	LCD_Command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | _displaycontrol);
	LL_mDelay(2);
}

//------------------------------------------------
void LCD_noCursor(void)
{
	_displaycontrol &= ~LCD_CURSORON;
	LCD_Command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | _displaycontrol);
	LL_mDelay(2);
}
//------------------------------------------------
void LCD_Blink(void)
{
	_displaycontrol |= LCD_BLINKON;
	LCD_Command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | _displaycontrol);
	LL_mDelay(2);
}
//------------------------------------------------
void LCD_noBlink(void)
{
	_displaycontrol &= ~LCD_BLINKON;
	LCD_Command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | _displaycontrol);
	LL_mDelay(2);
}

//------------------------------------------------
void LCD_init(void)
{
	//rs1;// add
	LL_mDelay(50);
	rs0;
	delay();// add
	LCD_WriteData(3);// b11
	e1;
	delay();
	e0;
	LL_mDelay(4);//LL_mDelay(1);
	LCD_WriteData(3);// b11
	e1;
	delay();
	e0;
	LL_mDelay(1);
	LCD_WriteData(3);// b11
	e1;
	delay();
	e0;
	LL_mDelay(1);
	// addition write data b10 // for LCD16x2 / LCD1602
	LCD_WriteData(2);// b10
	e1;
	delay();
	e0;
	LL_mDelay(1);
	// end addition
	LCD_Command(0x28);
	LL_mDelay(1);
	LCD_Command(0x28);
	LL_mDelay(1);
	LCD_Command(0x0C);//LCD_Command(0x0D);
	LL_mDelay(1);
	LCD_Command(0x06);
	//LL_mDelay(1);
	//LCD_Command(0x02);// set cursor position to zero
	//LL_mDelay(2);
	LL_mDelay(50);// 50 ms
}
/* USER CODE END 0 */
