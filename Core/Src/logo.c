/*
 * logo.c
 *
 *  Created on: Feb 14, 2023
 *      Author: as
 */
 
 /* Includes ------------------------------------------------------------------*/
#include "logo.h"
#include "lcd2004.h"


/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

char str_md[]             = {" Metal detector "};
char str_md_name[]        = {"Yashka & Mashka "};

//char str_md_version[]     = {"1.0.17 Fixed"};//  OLD SW VERSION
char str_md_version[]     = {"1.0.18 Fixed"};//  SW VERSION
char str_md_data[]        = {"04.11.2024"};//    DATA VERSION

//char str_md_made[]        = {"Made by"};
char str_md_made[]        = {"Software from"};
char str_family_name[]    = {"A.N.Z."};// 16 char

//char str_md_build[]       = {"Build PCB"};
//char str_md_build_date[]  = {"29.05.2024"};// Build PCB date

char str_md_mcu[]         = {"ARM Cortex-M4"};//char str_md_mcu[]         = {"MCU"};
char str_md_mcu_name[]    = {"STM32F401RCT6"};// MCU NAME

//char str_md_uid_str01[]    = {"Unique Device"};
//char str_md_uid_str02[]    = {"ID Protection"};

extern volatile uint8_t Target_flag;

/* USER CODE END PD */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void LogoVersion(void)
{
	// tmp variables
	char str01[16+1] = {0};
	char str02[16+1] = {0};
	
	// Step 1
	LCD_Clear();// clr
	LCD_SetPos(0, 0);
	sprintf(str01, "%s", str_md);
	LCD_String(str01);
	LCD_SetPos(0, 1);
	sprintf(str02, "%s", str_md_name);
	LCD_String(str02);
	LL_mDelay(2000);
	
	// Step 2
	LCD_Clear();// clr
	LCD_SetPos(0, 0);
	sprintf(str01, "%s", str_md_version);
	LCD_String(str01);
	LCD_SetPos(0, 1);
	sprintf(str02, "%s", str_md_data);
	LCD_String(str02);
	LL_mDelay(2000);
	
	// Step 3
	LCD_Clear();// clr
	//LCD_SetPos(0, 0);
	sprintf(str01, "%s", str_md_made);
	LCD_String(str01);
	LCD_SetPos(0, 1);
	sprintf(str02, "%s", str_family_name);
	LCD_String(str02);
	//LL_mDelay(1000);
	LL_mDelay(2000);
	
	// Step 4
	/*
	LCD_Clear();// clr
	LCD_SetPos(0, 0);
	sprintf(str01, "%s", str_md_build);
	LCD_String(str01);
	LCD_SetPos(0, 1);
	sprintf(str02, "%s", str_md_build_date);
	LCD_String(str02);
	LL_mDelay(1000);//LL_mDelay(2000);
	*/
	
	// Step 5
	LCD_Clear();// clr
	//LCD_SetPos(0, 0);
	sprintf(str01, "%s", str_md_mcu);
	LCD_String(str01);
	LCD_SetPos(0, 1);
	sprintf(str02, "%s", str_md_mcu_name);
	LCD_String(str02);
	LL_mDelay(2000);
	
	// Step 6
	//LCD_Clear();// clr
	////LCD_SetPos(0, 0);
	//sprintf(str01, "%s", str_md_uid_str01);
	//LCD_String(str01);
	//LCD_SetPos(0, 1);
	//sprintf(str02, "%s", str_md_uid_str02);
	//LCD_String(str02);
	//LL_mDelay(2000);
	
	LCD_Clear();// clr
	Target_flag = 1;// Update LCD
	
	// end
}


/* USER CODE END 0 */


