/*
 * menu.c
 *
 *  Created on: Jan 6, 2023
 *      Author: as
 */

 /* Includes ------------------------------------------------------------------*/
#include "lcd1602_menu.h"
#include "lcd2004.h"
#include "main.h"
#include "software_I2C.h"
#include "kt0803l_software_i2c.h"

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define DELL LL_mDelay(100)

volatile uint16_t menu_pos = 0;
volatile uint16_t menu_pos_old = 0;
volatile uint16_t sub_menu_pos = 0;
volatile uint8_t m_menu_flag = 0;// Main menu flag
volatile uint8_t s_menu_flag = 0;// Sub menu flag
volatile uint8_t p_menu_flag = 0;// Programm menu flag
// default
volatile uint8_t sw_default = 1;// switch menu 0 - Main menu
char str_vdi[] =  {"VDI\0"};// pos X 0-3 Y 0 VDI Value
char str_amp[] =  {"A\0"};// pos X 10 Y 0 Amplitude Value
// Frequency
//volatile uint8_t freq_chr_ind = 0;// 0-9
volatile uint8_t freq_chr_ind[5+1] = {0};// 0-9
volatile uint8_t freq_pos_cursor = 0;// 0-4 freq = 16000 or 18000
const char freq_chr[10] = {'0','1','2','3','4','5','6','7','8','9'};
volatile char freq_chr_set[5] = {'0','0','0','0','0'};
volatile uint16_t frequency_tmp = 0;
char str_Set_freq[]    =  {"Setup Frequency "};
char str_Search_freq_01[] =  {"Auto Search"};
char str_Search_freq_02[] =  {"Frequency"};
volatile uint8_t Set_freq_fl = 0;// flag
// GEB
char str_Set_GEB[]    =  {"Setup GEB"};
//char str_Backlight_sw[]    =  {"Backlight Switch"};
char str_ON[]    =  {" ON"};
char str_OFF[]   =  {"OFF"};
//
extern volatile int32_t Amp_diff;
extern volatile uint32_t TIM3_GEB_value;
extern volatile uint8_t Calib_auto;
extern volatile uint8_t Calib_flag;
//
//extern volatile uint8_t SW_Backlight;
extern volatile uint8_t SW_Signal_buff;
//
// extern
extern volatile uint16_t F_TX;
extern uint8_t Target_flag;
extern volatile uint16_t Sense_value;
extern volatile int32_t Amp_X;
extern volatile int32_t Amp_Y;
extern volatile int32_t Amp_B0_mV;
extern volatile int32_t Amp_B1_mV;
extern volatile int32_t Amp_B2_mV;
extern volatile int32_t Amp_B3_mV;
extern volatile int32_t Amp_mv;
//extern volatile uint8_t Battary_value = 0;

// Target Speed
char str_Set_speed[]   =  {"Setup Speed "};
//char str_Hz[]          =  {"Hz"};
volatile uint16_t TargetSpeed;
#define NB_Speed 4 // Max value 4
volatile uint8_t Speed_index = 2;// default T_32Hz
extern volatile uint16_t T_16Hz;// Speed_index 0
extern volatile uint16_t T_24Hz;// Speed_index 1
extern volatile uint16_t T_32Hz;// Speed_index 2
extern volatile uint16_t T_48Hz;// Speed_index 3
extern volatile uint16_t T_64Hz;// Speed_index 4
extern volatile uint16_t T_H_CNT;
// KT0803L FM Transmitter
extern float KT0803L_Frequency;
//extern uint16_t KT0803L_Channel;
extern uint16_t KT0803L_Channel_tmp;
extern uint8_t PA_BIAS_enable;
volatile uint16_t Freq_FM_x100;
extern uint16_t KT0803L_ON_flag;// 1 - ON, 0 - OFF. Enable and disable chip KT0803L. 
// RX mode
extern volatile uint8_t RX_Sample_bit_offset;// 0 or 1. RX bit offset: 1
// Mask Ferrum
extern volatile uint8_t Mask_Ferrum_flag;

// Main menu PD
char *mm1[MAIN_NENU_NB] = { 
"Barrier \0", 
"GEB \0", 
"Mask Ferrum \0", 
"Signal Buffering", 
"RX Sample Offset", // menu pos 50
"Coil bal \0", 
"TX Frequency \0", 
"Target Speed \0",
"FM Transmitter" 
//"Ferrite calib \0", 
//"Backlight \0" 
};

char SAVED_in_EEPROM[] = {"SAVED in EEPROM"};

char str01[16+1] = {0};
char str02[16+1] = {0};
//char Enter_menu[] = {"Enter in Menu   "};
//char Exit_menu[] =  {"      Exit      "};
//char End_menu[]  =  {"End Menu        "};

/* USER CODE END PD */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void SetFrequency(void)
{
	// menu_pos 710
	volatile uint8_t pos = 0;
	volatile uint8_t ind = 0;
	// flag program
	Set_freq_fl = 1;
	LCD_Clear();// clr
	// ON Cursor
	LCD_Cursor();
	//LCD_Blink();
	// Set cursor in position 0
	freq_pos_cursor = 0;
	// Save index old freq
	char freq_chr_old[5+1] = {'0'};// old freq
	volatile uint8_t scroll_pos_cnt = 0;
	sprintf(freq_chr_old, "%d", F_TX);
	volatile uint8_t i;
	for(i = 0; i < 5; i++){
		// copy old index
		if (F_TX > 9999){
			freq_chr_ind[i] = freq_chr_old[i] - 48;
			//printf("old F_TX index: %d c: %d \n", i, freq_chr_old[i]);// debug
		} else{
			freq_chr_ind[i+1] = freq_chr_old[i] - 48;
			//printf("old F_TX index: %d c: %d \n", i, freq_chr_old[i]);// debug
		}		
	}	
	//
	while (Set_freq_fl)// while program	
	{
		// line 1
		LCD_SetPos(0, 0);
		sprintf(str01, "%s", str_Set_freq);
		LCD_String(str01);
		// line 2
		// scroll position to viev freq LCD
		if (scroll_pos_cnt < 5){
			scroll_pos_cnt += 1;
			freq_pos_cursor += 1;
			if (freq_pos_cursor > 4) { freq_pos_cursor = 0; }// 0-4
		}		
		// end scroll
		if (Button_UP() && Set_freq_fl) {// ++ 
			//freq_chr_ind += 1;
			freq_chr_ind[freq_pos_cursor] += 1;
			//if (freq_chr_ind > 9) { freq_chr_ind = 0; }// 0-255
			if (freq_chr_ind[freq_pos_cursor] > 9) { freq_chr_ind[freq_pos_cursor] = 0; }// 0-255
			//if ((freq_pos_cursor == 0) && (freq_chr_ind > 2)) { freq_chr_ind = 0; }// position 0
			if ((freq_pos_cursor == 0) && (freq_chr_ind[freq_pos_cursor] > 2)) { freq_chr_ind[freq_pos_cursor] = 0; }// position 0
			// debug
			//printf("Press button UP\n");
			DELL;
		}
		if (Button_DOWN() && Set_freq_fl) {// -- 
			//freq_chr_ind -= 1;
			freq_chr_ind[freq_pos_cursor] -= 1;
			//if (freq_chr_ind > 9) { freq_chr_ind = 9; }// 0-255
			if (freq_chr_ind[freq_pos_cursor] > 9) { freq_chr_ind[freq_pos_cursor] = 9; }// 0-255
			//if ((freq_pos_cursor == 0) && (freq_chr_ind > 2)) { freq_chr_ind = 2; }// position 0
			if ((freq_pos_cursor == 0) && (freq_chr_ind[freq_pos_cursor] > 2)) { freq_chr_ind[freq_pos_cursor] = 2; }// position 0
			// debug
			//printf("Press button DOWN\n");
			DELL;
		}
		if (Button_RIGHT() && Set_freq_fl) {// --> 
			freq_pos_cursor += 1;
			if (freq_pos_cursor > 4) { freq_pos_cursor = 0; }// 0-4
			// debug
			//printf("Press button RIGHT\n");
			DELL;
		}
		if (Button_LEFT() && Set_freq_fl) {// <-- 
			freq_pos_cursor -= 1;
			if (freq_pos_cursor > 4) { freq_pos_cursor = 4; }// 0-4
			// debug
			//printf("Press button LEFT\n");
			DELL;
		}
		// Setup 
		pos = freq_pos_cursor;
		//ind = freq_chr_ind;
		ind = freq_chr_ind[freq_pos_cursor];
		freq_chr_set[pos] = freq_chr[ind];
		//freq_chr_set[freq_pos_cursor] = freq_chr[freq_chr_ind];
		// debug
		LCD_SetPos(10, 1);
		sprintf(str02, "%d", pos);
		LCD_String(str02);
		LCD_SetPos(13, 1);
		sprintf(str02, "%d", ind);
		LCD_String(str02);
		// end debug
		// set data tmp
		frequency_tmp = (uint16_t)((freq_chr_set[0] - 48) * 10000);
		frequency_tmp += (uint16_t)((freq_chr_set[1] - 48) * 1000);
		frequency_tmp += (uint16_t)((freq_chr_set[2] - 48) * 100);
		frequency_tmp += (uint16_t)((freq_chr_set[3] - 48) * 10);
		frequency_tmp += (uint16_t)(freq_chr_set[4] - 48);
		if (frequency_tmp >= 10000) {
			LCD_SetPos(0, 1);
		} else{
			LCD_SetPos(0, 1);
			LCD_String(" ");
			LCD_SetPos(1, 1);
		}		
		sprintf(str02, "%d", frequency_tmp);
		LCD_String(str02);
		LCD_SetPos(6, 1);
		LCD_String("Hz");
		LCD_SetPos(pos, 1);
		//
		if (Button_OK() && Set_freq_fl) {// Setup and save Frequency
			SetFrequencyTX(frequency_tmp);
			// function saved in eeprom
			i2c_init();
			EEPROM_Write16(EEPROM_ADDR_FREQ_TX, frequency_tmp);//EEPROM_Write16(0x0020, frequency_tmp);
			LCD_Clear();// clr
			sprintf(str01, "%s", SAVED_in_EEPROM);//sprintf(str01, "SAVED in EEPROM");
			LCD_String(str01);
			LL_mDelay(1000);// 1000 ms
			// end eeprom function
			menu_pos = 70;
			Set_freq_fl = 0;// clr flag
			// debug
			//printf("frequency_tmp: %lu menu_pos = %lu\n", (uint32_t)frequency_tmp, (uint32_t)menu_pos);// debug
			// debug
			//printf("Press button OK\n");
			p_menu_flag = 0;// clr Programm menu flag
			s_menu_flag = 0;// clr flag Sub menu
			m_menu_flag = 1;// set flag Main !!!
			DELL;
		}
		if (Button_MENU() && Set_freq_fl) {// Setup Frequency No save
			SetFrequencyTX(frequency_tmp);
			menu_pos = 70;
			Set_freq_fl = 0;// clr flag
			// debug
			//printf("Press button MENU\n");
			p_menu_flag = 0;// clr Programm menu flag
			s_menu_flag = 0;// clr flag Sub menu
			m_menu_flag = 1;// set flag Main !!!
			DELL;
		}
		LL_mDelay(100);//LL_mDelay(250);
	}// end while
	// OFF Cursor
	LCD_noCursor();
	//LCD_noBlink();
	Calib_auto = 0;// Auto Calibration OFF - Corrections VDI value
	Calib_flag = 1;// Set flag - Get Calibration Samples
}

// KT0803L FM Transmitter 
void FM_SetFrequency(void)
{
	// menu_pos 910
	volatile uint8_t pos = 0;
	volatile uint8_t ind = 0;
	// flag program
	Set_freq_fl = 1;
	LCD_Clear();// clr
	// ON Cursor
	LCD_Cursor();
	//LCD_Blink();
	// Set cursor in position 0
	freq_pos_cursor = 0;
	// Save index old freq
	char freq_chr_old[5+1] = {'0'};// old freq
	volatile uint8_t scroll_pos_cnt = 0;
	Freq_FM_x100 = round(KT0803L_Frequency * 100);
	sprintf(freq_chr_old, "%d", Freq_FM_x100);
	volatile uint8_t i;
	for(i = 0; i < 5; i++){
		// copy old index
		if (Freq_FM_x100 > 9999){
			freq_chr_ind[i] = freq_chr_old[i] - 48;
			//printf("old Freq_FM_x100 index: %d c: %d \n", i, freq_chr_old[i]);// debug
		} else{
			freq_chr_ind[i+1] = freq_chr_old[i] - 48;
			//printf("old Freq_FM_x100 index: %d c: %d \n", i, freq_chr_old[i]);// debug
		}		
	}	
	//
	while (Set_freq_fl)// while program	
	{
		// line 1
		LCD_SetPos(0, 0);
		sprintf(str01, "%s", str_Set_freq);
		LCD_String(str01);
		// line 2
		// scroll position to viev freq LCD
		if (scroll_pos_cnt < 5){
			scroll_pos_cnt += 1;
			freq_pos_cursor += 1;
			if (freq_pos_cursor > 4) { freq_pos_cursor = 0; }// 0-4
		}		
		// end scroll
		if (Button_UP() && Set_freq_fl) {// ++ 
			//freq_chr_ind += 1;
			freq_chr_ind[freq_pos_cursor] += 1;
			//if (freq_chr_ind > 9) { freq_chr_ind = 0; }// 0-255
			if (freq_chr_ind[freq_pos_cursor] > 9) { freq_chr_ind[freq_pos_cursor] = 0; }// 0-255
			//if ((freq_pos_cursor == 0) && (freq_chr_ind > 2)) { freq_chr_ind = 0; }// position 0
			if ((freq_pos_cursor == 0) && (freq_chr_ind[freq_pos_cursor] > 1)) { freq_chr_ind[freq_pos_cursor] = 0; }// position 0
			// debug
			//printf("Press button UP\n");
			DELL;
		}
		if (Button_DOWN() && Set_freq_fl) {// -- 
			//freq_chr_ind -= 1;
			freq_chr_ind[freq_pos_cursor] -= 1;
			//if (freq_chr_ind > 9) { freq_chr_ind = 9; }// 0-255
			if (freq_chr_ind[freq_pos_cursor] > 9) { freq_chr_ind[freq_pos_cursor] = 9; }// 0-255
			//if ((freq_pos_cursor == 0) && (freq_chr_ind > 2)) { freq_chr_ind = 2; }// position 0
			if ((freq_pos_cursor == 0) && (freq_chr_ind[freq_pos_cursor] > 1)) { freq_chr_ind[freq_pos_cursor] = 1; }// position 0
			// debug
			//printf("Press button DOWN\n");
			DELL;
		}
		if (Button_RIGHT() && Set_freq_fl) {// --> 
			freq_pos_cursor += 1;
			if (freq_pos_cursor > 4) { freq_pos_cursor = 0; }// 0-4
			// debug
			//printf("Press button RIGHT\n");
			DELL;
		}
		if (Button_LEFT() && Set_freq_fl) {// <-- 
			freq_pos_cursor -= 1;
			if (freq_pos_cursor > 4) { freq_pos_cursor = 4; }// 0-4
			// debug
			//printf("Press button LEFT\n");
			DELL;
		}
		// Setup 
		pos = freq_pos_cursor;
		//ind = freq_chr_ind;
		ind = freq_chr_ind[freq_pos_cursor];
		freq_chr_set[pos] = freq_chr[ind];
		//freq_chr_set[freq_pos_cursor] = freq_chr[freq_chr_ind];
		// debug
		LCD_SetPos(11, 1);//LCD_SetPos(10, 1);
		sprintf(str02, "%d", pos);
		LCD_String(str02);
		LCD_SetPos(14, 1);//LCD_SetPos(13, 1);
		sprintf(str02, "%d", ind);
		LCD_String(str02);
		// end debug
		// set data tmp
		frequency_tmp = (uint16_t)((freq_chr_set[0] - 48) * 10000);
		frequency_tmp += (uint16_t)((freq_chr_set[1] - 48) * 1000);
		frequency_tmp += (uint16_t)((freq_chr_set[2] - 48) * 100);
		frequency_tmp += (uint16_t)((freq_chr_set[3] - 48) * 10);
		frequency_tmp += (uint16_t)(freq_chr_set[4] - 48);
		if (frequency_tmp >= 10000) {
			LCD_SetPos(0, 1);
		} else{
			LCD_SetPos(0, 1);
			LCD_String(" ");
			LCD_SetPos(1, 1);
		}	
		Freq_FM_x100 = frequency_tmp;// set !!!
		KT0803L_Frequency = (float)Freq_FM_x100 * 0.01;// float divide by 100 
		sprintf(str02, "%d", frequency_tmp);
		//sprintf(str02, "%.2f", KT0803L_Frequency);
		LCD_String(str02);
		LCD_SetPos(5, 1);//LCD_SetPos(6, 1);
		LCD_String("0 kHz");//LCD_String("Hz");
		LCD_SetPos(pos, 1);
		//
		if (Button_OK() && Set_freq_fl) {// Setup and save Frequency
			// Setup Frequency in KT0803L
			KT0803L_i2c_init();
			KT0803L_SetFrequency(KT0803L_Frequency, PA_BIAS_enable);// float and uint8_t 
			// function saved in eeprom
			KT0803L_Channel_tmp = KT0803L_FrequencyToChannel(KT0803L_Frequency);// uint16_t 
			LL_mDelay(100);// 100 ms
			i2c_init();
			EEPROM_Write16(EEPROM_ADDR_KT0803L_FM_CANNEL, KT0803L_Channel_tmp);// address 0x0010 
			LCD_Clear();// clr
			sprintf(str01, "%s", SAVED_in_EEPROM);//sprintf(str01, "SAVED in EEPROM");
			LCD_String(str01);
			LL_mDelay(1000);// 1000 ms
			// end eeprom function
			menu_pos = 90;
			Set_freq_fl = 0;// clr flag
			// debug
			//printf("Freq_FM_x100: %lu menu_pos = %lu\n", (uint32_t)Freq_FM_x100, (uint32_t)menu_pos);// debug
			// debug
			//printf("Press button OK\n");
			p_menu_flag = 0;// clr Programm menu flag
			s_menu_flag = 0;// clr flag Sub menu
			m_menu_flag = 1;// set flag Main !!!
			DELL;
		}
		if (Button_MENU() && Set_freq_fl) {// Setup Frequency No save
			// Setup Frequency in KT0803L
			KT0803L_i2c_init();
			KT0803L_SetFrequency(KT0803L_Frequency, PA_BIAS_enable);// float and uint8_t 
			menu_pos = 90;
			Set_freq_fl = 0;// clr flag
			// debug
			//printf("Press button MENU\n");
			p_menu_flag = 0;// clr Programm menu flag
			s_menu_flag = 0;// clr flag Sub menu
			m_menu_flag = 1;// set flag Main !!!
			DELL;
		}
		LL_mDelay(100);//LL_mDelay(250);
	}// end while
	// OFF Cursor
	LCD_noCursor();
	//LCD_noBlink(); 
}


// Target Speed
void GetTargetSpeed(void)
{	
	TargetSpeed = 100000 / T_H_CNT;	
}

/*
void SetupTargetSpeed(uint8_t Sw_Index)
{
	switch (Sw_Index) {		
		case 0:
		T_H_CNT = T_16Hz;
		break;		
		case 1:
		T_H_CNT = T_24Hz;
		break;		
		case 2:
		T_H_CNT = T_32Hz;// Default 32 Hz.
		break;		
		case 3:
		T_H_CNT = T_48Hz;
		break;		
		case 4:
		T_H_CNT = T_64Hz;
		break;		
		default:
		T_H_CNT = T_32Hz;// Default 32 Hz.
		break;
	}
}
*/

void SelectTargetSpeed(void)
{
	// menu_pos 810
	// flag program
	volatile uint8_t Set_Speed_fl = 1;
	LCD_Clear();// clr
	// ON Cursor
	LCD_Cursor();
	// while
	while (Set_Speed_fl)// while program
	{
		// line 1
		LCD_SetPos(0, 0);
		LCD_String("Select Up / Down");
		// line 2 - Get value speed
		GetTargetSpeed();
		LCD_SetPos(0, 1);
		sprintf(str01, "%lu", (uint32_t)TargetSpeed);
		LCD_String(str01);
		LCD_SetPos(6, 1);
		LCD_String("Hz");
		//
		// Buttons
		if (Button_UP() && Set_Speed_fl) {// ++ 
			Speed_index += 1;
			if (Speed_index > (uint8_t)NB_Speed) { Speed_index = 0; }// 0-4
			//SetupTargetSpeed(Speed_index);
			Match_Speed_Filter(Speed_index);
			DELL;
		}
		if (Button_DOWN() && Set_Speed_fl) {// -- 
			Speed_index -= 1;
			if (Speed_index > (uint8_t)NB_Speed) { Speed_index = (uint8_t)NB_Speed; }// 0-4
			//SetupTargetSpeed(Speed_index);
			Match_Speed_Filter(Speed_index);
			DELL;
		}
		// index value
		LCD_SetPos(10, 1);
		sprintf(str02, "%d", Speed_index);
		LCD_String(str02);
		//
		if (Button_OK() && Set_Speed_fl) {// Setup and save
			//SetupTargetSpeed(Speed_index);
			Match_Speed_Filter(Speed_index);
			// function saved in eeprom
			i2c_init();
			EEPROM_Write16(EEPROM_ADDR_FILTER_SPEED, (uint16_t)Speed_index);// addr 0x0040
			LCD_Clear();// clr
			sprintf(str01, "%s", SAVED_in_EEPROM);//sprintf(str01, "SAVED in EEPROM");
			LCD_String(str01);
			LL_mDelay(1000);// 1000 ms
			// end eeprom function
			menu_pos = 80;
			p_menu_flag = 0;// clr Programm menu flag
			s_menu_flag = 0;// clr flag Sub menu
			m_menu_flag = 1;// set flag Main !!!
			Set_Speed_fl = 0;// clr flag
			DELL;
		}
		if (Button_MENU() && Set_Speed_fl) {// No Setup No save
			menu_pos = 80;
			p_menu_flag = 0;// clr Programm menu flag
			s_menu_flag = 0;// clr flag Sub menu
			m_menu_flag = 1;// set flag	!!!
			Set_Speed_fl = 0;// clr flag
			DELL;
		}
		if (Button_LEFT() && Set_Speed_fl) {// No Setup No save
			menu_pos = 80;
			p_menu_flag = 0;// clr Programm menu flag
			s_menu_flag = 0;// clr flag sub menu
			m_menu_flag = 1;// set flag	!!!
			Set_Speed_fl = 0;// clr flag
			DELL;
		}
		
		// Pause
		LL_mDelay(100);
	}// end while
	// OFF Cursor
	LCD_noCursor();
}
// END Target Speed


// Ground balance coarse adjustment
void SetGEB(void)
{
	// menu_pos 210
	//volatile uint8_t TX_index = 0;
	volatile uint8_t TX_index = (uint8_t)(TIM2->CCR3);// Read old value
	volatile uint8_t TX_index_old = (uint8_t)(TIM2->CCR3);// CMP Changes
	// flag program
	volatile uint8_t Set_GEB_fl = 1;
	LCD_Clear();// clr
	// ON Cursor
	LCD_Cursor();
	//LCD_Blink();
	// while 1
	while (Set_GEB_fl)// while program
	{
		// line 1
		LCD_SetPos(0, 0);
		LCD_String("Amp Diff");
		// value
		LCD_SetPos(10, 0);
		LCD_String("      ");// clr - len 6
		LCD_SetPos(10, 0);//LCD_SetPos(11, 0);
		//corr if (Amp_diff > 0) { LCD_SetPos(11, 0); }
		sprintf(str01, "%ld", Amp_diff);		
		LCD_String(str01);
		// line 2
		if (Button_UP() && Set_GEB_fl) {// ++ 
			TX_index += 1;
			if (TX_index > NB_GEB) { TX_index = 0; }// 0-7
			Setup_GB_TIM1_CH1_CNT(0.00f, 0, TX_index, 1);
			DELL;
		}
		if (Button_DOWN() && Set_GEB_fl) {// -- 
			TX_index -= 1;
			if (TX_index > NB_GEB) { TX_index = NB_GEB; }// 0-7
			Setup_GB_TIM1_CH1_CNT(0.00f, 0, TX_index, 1);
			DELL;
		}
		// value
		LCD_SetPos(1, 1);
		sprintf(str02, "%d", TX_index);
		LCD_String(str02);
		LCD_SetPos(3, 1);
		LCD_String("= TX index");
		LCD_SetPos(1, 1);
		//
		if (Button_OK() && Set_GEB_fl) {// Setup and save
			Setup_GB_TIM1_CH1_CNT(0.00f, 0, TX_index, 1);
			// function saved in eeprom
			i2c_init();
			EEPROM_Write16(EEPROM_ADDR_GEB, (uint16_t)TX_index);// addr 0x0030
			LCD_Clear();// clr
			sprintf(str01, "%s", SAVED_in_EEPROM);//sprintf(str01, "SAVED in EEPROM");
			LCD_String(str01);
			LL_mDelay(1000);// 1000 ms
			// end eeprom function
			menu_pos = 20;
			p_menu_flag = 0;// clr Programm menu flag
			s_menu_flag = 0;// clr flag Sub menu
			m_menu_flag = 1;// set flag Main !!!
			Set_GEB_fl = 0;// clr flag
			DELL;
		}
		if (Button_MENU() && Set_GEB_fl) {// No Setup No save
			//Setup_GB_TIM1_CH1_CNT(0.00f, 0, TX_index, 1);
			menu_pos = 20;
			p_menu_flag = 0;// clr Programm menu flag
			s_menu_flag = 0;// clr flag Sub menu
			m_menu_flag = 1;// set flag	!!!
			Set_GEB_fl = 0;// clr flag
			DELL;
		}
		if (Button_LEFT() && Set_GEB_fl) {// No Setup No save
			//Setup_GB_TIM1_CH1_CNT(0.00f, 0, TX_index, 1);
			menu_pos = 20;
			p_menu_flag = 0;// clr Programm menu flag
			s_menu_flag = 0;// clr flag sub menu
			m_menu_flag = 1;// set flag	!!!
			Set_GEB_fl = 0;// clr flag
			DELL;
		}
		// CMP Changes
		if (TX_index != TX_index_old) {
			Calib_auto = 0;// Auto Calibration OFF - Corrections VDI value
			Calib_flag = 1;// Set flag - Get Calibration Samples
		}
		// Pause
		LL_mDelay(100);
	}// end while
	// OFF Cursor
	LCD_noCursor();
	//LCD_noBlink();
}



void MenuVDI(int16_t vdi_value, int32_t amp_value)
{
	LCD_Clear();// clr
	// line 1
	//LCD_SetPos(0, 0);
	sprintf(str01, "%s", str_vdi);
	LCD_String(str01);
	// ':' pos 3
	//LCD_SetPos(3, 0);
	//LCD_String(":");
	// Value VDI pos 5-7
	LCD_SetPos(4, 0);//LCD_SetPos(5, 0);
	LCD_String("   ");// space
	if (vdi_value > 99) { LCD_SetPos(4, 0); } //if (vdi_value > 99) { LCD_SetPos(5, 0); } 
	if ( (vdi_value <= 99) && (vdi_value > 0) ) { LCD_SetPos(5, 0); } //if ( (vdi_value <= 99) && (vdi_value > 0) ) { LCD_SetPos(6, 0); } 
	if ( vdi_value < (-99) ) { LCD_SetPos(4-1, 0); } //if ( vdi_value < (-99) ) { LCD_SetPos(5-1, 0); } 
	if ( (vdi_value >= (-99)) && (vdi_value < 0) ) { LCD_SetPos(5-1, 0); } //if ( (vdi_value >= (-99)) && (vdi_value < 0) ) { LCD_SetPos(6-1, 0); } 
	if ( (vdi_value > (-10) && vdi_value < 0) ) { LCD_SetPos(6-1, 0); } //if ( (vdi_value > (-10) && vdi_value < 0) ) { LCD_SetPos(7-1, 0); } 
	if ( (vdi_value == 0) || (vdi_value < 10 && vdi_value > 0) ) { LCD_SetPos(6, 0); } //if ( (vdi_value == 0) || (vdi_value < 10 && vdi_value > 0) ) { LCD_SetPos(7, 0); } 
	sprintf(str01, "%d", vdi_value);
	LCD_String(str01);
	// AMP Value pos 9-10, pos 11-15.
	LCD_SetPos(9, 0);//LCD_SetPos(10, 0);
	sprintf(str01, "%s", str_amp);
	LCD_String(str01);
	LCD_SetPos(10, 0);//LCD_SetPos(11, 0);
	if (amp_value > 0) { LCD_SetPos(11, 0); }
	sprintf(str01, "%ld", amp_value);
	LCD_String(str01);
	// Line 2 - VDI scale.
	// - VDI
	//if (vdi_value < 0) { LCD_SetPos(0, 1); LCD_String("BLACK"); } 
	// + VDI
	//if (vdi_value > 0) { LCD_SetPos(5, 1); LCD_String("** COLOR **"); } 
	//
	// VDI-metr
	// The first 4 sectors of VDI are less than zero - ferrous metal. One sector - a step of 30 degrees.
	// Only 4 sectors - a maximum of -120 degrees.
	// 5 - 16 sectors. VDI greater than zero - non-ferrous metal. One sector - a step of 10 degrees. 
	// Only 12 sectors - only +120 degrees.
	// CLR 2 lines LCD
	LCD_SetPos(0, 1); LCD_String("                ");
	// Ferrum 
	if (vdi_value < -90) { LCD_SetPos(0, 1); LCD_String("F"); }    // Sector 1
	if ((vdi_value >= -90) && (vdi_value < -60)) { LCD_SetPos(1, 1); LCD_String("J"); }  // Sector 2
	if ((vdi_value >= -60) && (vdi_value < -30)) { LCD_SetPos(2, 1); LCD_String("J"); }  // Sector 3
	if ((vdi_value >= -30) && (vdi_value < 0)) { LCD_SetPos(3, 1); LCD_String("?"); }    // Sector 4
	// Non-Ferrum
	// Au - Aurum :-)
	if ((vdi_value >= 0) && (vdi_value < 10)) { LCD_SetPos(4, 1); LCD_String("Z"); }     // Sector 5
	if ((vdi_value >= 10) && (vdi_value < 20)) { LCD_SetPos(5, 1); LCD_String("Z"); }    // Sector 6	
	// Ag - Argentum
	if ((vdi_value >= 20) && (vdi_value < 30)) { LCD_SetPos(6, 1); LCD_String("S"); }    // Sector 7
	if ((vdi_value >= 30) && (vdi_value < 40)) { LCD_SetPos(7, 1); LCD_String("S"); }    // Sector 8
	// Other metal
	if ((vdi_value >= 40) && (vdi_value < 50)) { LCD_SetPos(8, 1); LCD_String("*"); }    // Sector 9
	if ((vdi_value >= 50) && (vdi_value < 60)) { LCD_SetPos(9, 1); LCD_String("*"); }    // Sector 10
	if ((vdi_value >= 60) && (vdi_value < 70)) { LCD_SetPos(10, 1); LCD_String("*"); }   // Sector 11
	if ((vdi_value >= 70) && (vdi_value < 80)) { LCD_SetPos(11, 1); LCD_String("*"); }   // Sector 12
	// Copper !!! 80 - 99
	if ((vdi_value >= 80) && (vdi_value < 90)) { LCD_SetPos(12, 1); LCD_String("M"); }   // Sector 13
	if ((vdi_value >= 90) && (vdi_value < 100)) { LCD_SetPos(13, 1); LCD_String("M"); }  // Sector 14
	// Other metal
	if ((vdi_value >= 100) && (vdi_value < 110)) { LCD_SetPos(14, 1); LCD_String("+"); } // Sector 15
	if (vdi_value >= 110) { LCD_SetPos(15, 1); LCD_String("+"); }                        // Sector 16
	// end VDI-metr	
}

void LCD_Sense(void)// Sense value Fast Setup
{
	//char str_Sense[8] = {"Barrier"};
	LCD_Clear();
	//LCD_SetPos(0, 0);
	sprintf(str01, "%s", mm1[0]);//sprintf(str01, "%s", str_Sense);
	LCD_String(str01);
	// value
	LCD_SetPos(0, 1);//LCD_SetPos(8, 0);
	sprintf(str02, "%d", Sense_value);
	LCD_String(str02);
	//LL_mDelay(100);// 100 ms
}

// Barrier
void SetSense(void)
{
	LCD_Clear();
	// line 1
	//LCD_SetPos(0, 0);
	LCD_String("Select Up / Down");
	volatile uint8_t fl = 1;
	while (fl) 
	{
	// line 2
	LCD_SetPos(0, 1);
	LCD_String("     ");// clr old value
	if (Sense_value < 10) { LCD_SetPos(1, 1); } else { LCD_SetPos(0, 1); } 
	sprintf(str02, "%d", Sense_value);
	LCD_String(str02);
	//
	if (Button_UP()) {
		if (Sense_value >= 10) {
			Sense_value += 5; 
		} else {
			Sense_value += 1; 
		}
		if (Sense_value > 1000) { Sense_value = 1000; }
	}
	if (Button_DOWN()) {
		if (Sense_value > 10) {
			Sense_value -= 5;
		} else {
			Sense_value -= 1;
		}
		if (Sense_value < 1) { Sense_value = 1; }
	}
	if (Button_OK()) {
		// function saved in eeprom
		i2c_init();
		EEPROM_Write16(EEPROM_ADDR_SENSE, Sense_value);// addr 0x0050
		LCD_Clear();// clr
		sprintf(str01, "%s", SAVED_in_EEPROM);//sprintf(str01, "SAVED in EEPROM");
		LCD_String(str01);
		LL_mDelay(1000);// 1000 ms
		// end eeprom function
		// Exit in Main menu
		menu_pos = 10;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	if (Button_MENU()) {
		// Exit in Main menu
		menu_pos = 10;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	if (Button_LEFT()) {
		// Exit in Main menu
		menu_pos = 10;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	DELL;// pause 100 ms 
	}// while 
}

// Switch Buffering Mode
void SetBufferingMode(void)
{
	LCD_Clear();
	// line 1
	//LCD_SetPos(0, 0);
	LCD_String("Select Up / Down");
	volatile uint8_t fl = 1;
	while (fl) 
	{
	// line 2
	LCD_SetPos(0, 1);
	sprintf(str02, "Mode: %d", SW_Signal_buff);
	LCD_String(str02);
	//
	if (Button_UP()) {
		SW_Signal_buff = 1 - SW_Signal_buff;// switch 0 or 1
	}
	if (Button_DOWN()) {
		SW_Signal_buff = 1 - SW_Signal_buff;// switch 0 or 1
	}
	if (Button_OK()) {
		// function saved in eeprom
		i2c_init();
		SW_Signal_buff &= 0x01;// by mask 1 
		EEPROM_Write16(EEPROM_ADDR_SW_BUFFER_MODE, (uint16_t)SW_Signal_buff);// addr 0x0060
		LCD_Clear();// clr
		sprintf(str01, "%s", SAVED_in_EEPROM);
		LCD_String(str01);
		LL_mDelay(1000);// 1000 ms
		// end eeprom function
		// Exit in Main menu
		menu_pos = 40;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	if (Button_MENU()) {
		// Exit in Main menu
		menu_pos = 40;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	if (Button_LEFT()) {
		// Exit in Main menu
		menu_pos = 40;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	DELL;// pause 100 ms 
	}// while 
}

// RX Sample bit offset
void SetRXbit()
{
	LCD_Clear();
	// line 1
	//LCD_SetPos(0, 0);
	LCD_String("Select Up / Down");
	volatile uint8_t fl = 1;
	while (fl) 
	{
	// line 2
	LCD_SetPos(0, 1);
	sprintf(str02, "RX offset: %d", RX_Sample_bit_offset);
	LCD_String(str02);
	//
	if (Button_UP()) {
		RX_Sample_bit_offset = 1 - RX_Sample_bit_offset;// switch 0 or 1
	}
	if (Button_DOWN()) {
		RX_Sample_bit_offset = 1 - RX_Sample_bit_offset;// switch 0 or 1
	}
	if (Button_OK()) {
		// function saved in eeprom
		i2c_init();
		RX_Sample_bit_offset &= 0x01;// by mask 1 
		EEPROM_Write16(EEPROM_ADDR_RX_MODE, (uint16_t)RX_Sample_bit_offset);// addr 0x0080
		LCD_Clear();// clr
		sprintf(str01, "%s", SAVED_in_EEPROM);
		LCD_String(str01);
		LL_mDelay(1000);// 1000 ms
		// end eeprom function
		// Exit in Main menu
		menu_pos = 50;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	if (Button_MENU()) {
		// Exit in Main menu
		menu_pos = 50;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	if (Button_LEFT()) {
		// Exit in Main menu
		menu_pos = 50;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	DELL;// pause 100 ms 
	}// while 
}

// Mask Ferrum
void SetMaskFerrum()
{
	LCD_Clear();
	// line 1
	//LCD_SetPos(0, 0);
	LCD_String("Select Up / Down");
	volatile uint8_t fl = 1;
	while (fl) 
	{
	// line 2
	LCD_SetPos(0, 1);
	sprintf(str02, "Mode: %d", Mask_Ferrum_flag);
	LCD_String(str02);
	//
	if (Button_UP()) {
		Mask_Ferrum_flag = 1 - Mask_Ferrum_flag;// switch 0 or 1
	}
	if (Button_DOWN()) {
		Mask_Ferrum_flag = 1 - Mask_Ferrum_flag;// switch 0 or 1
	}
	if (Button_OK()) {
		// function saved in eeprom
		i2c_init();
		Mask_Ferrum_flag &= 0x01;// by mask 1 
		EEPROM_Write16(EEPROM_ADDR_FERRUM_MASK, (uint16_t)Mask_Ferrum_flag);// addr 0x0090
		LCD_Clear();// clr
		sprintf(str01, "%s", SAVED_in_EEPROM);
		LCD_String(str01);
		LL_mDelay(1000);// 1000 ms
		// end eeprom function
		// Exit in Main menu
		menu_pos = 30;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	if (Button_MENU()) {
		// Exit in Main menu
		menu_pos = 30;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	if (Button_LEFT()) {
		// Exit in Main menu
		menu_pos = 30;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	DELL;// pause 100 ms 
	}// while 
}

// FM transmitter. Chip mode - Power On / Power Off
void KT0803L_OnOff()
{
	LCD_Clear();
	// line 1
	//LCD_SetPos(0, 0);
	LCD_String("Select Up / Down");
	volatile uint8_t fl = 1;
	while (fl) 
	{
	// line 2
	LCD_SetPos(0, 1);
	if (KT0803L_ON_flag == 1){
		sprintf(str02, "%s", str_ON);
		LCD_String(str02);
	} else {
		sprintf(str02, "%s", str_OFF);
		LCD_String(str02);
	}
	//
	if (Button_UP()) {
		KT0803L_ON_flag = 1 - KT0803L_ON_flag;// switch 0 or 1
	}
	if (Button_DOWN()) {
		KT0803L_ON_flag = 1 - KT0803L_ON_flag;// switch 0 or 1
	}
	if (Button_OK()) {
		// function saved in eeprom
		i2c_init();
		KT0803L_ON_flag &= 0x01;// by mask 1 
		EEPROM_Write16(EEPROM_ADDR_KT0803L_ON_OFF, KT0803L_ON_flag);// addr 0x00B0, uint16_t value 
		LCD_Clear();// clr
		sprintf(str01, "%s", SAVED_in_EEPROM);
		LCD_String(str01);
		LL_mDelay(1000);// 1000 ms
		// end eeprom function
		// Exit in Main menu
		menu_pos = 90;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
		// PB2 pin - set bit / clr bit 
		if (KT0803L_ON_flag == 1) {
			KT0803L_ON;
			LL_mDelay(500);// 500 ms
			KT0803L_Init();
		} else {
			KT0803L_OFF;
		}
		//
	}
	if (Button_MENU()) {
		// Exit in Main menu
		menu_pos = 90;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
		// PB2 pin - set bit / clr bit 
		if (KT0803L_ON_flag == 1) {
			KT0803L_ON;
			LL_mDelay(500);// 500 ms
			KT0803L_Init();
		} else {
			KT0803L_OFF;
		}
		//
	}
	if (Button_LEFT()) {
		// Exit in Main menu
		menu_pos = 90;
		fl = 0;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		//
	}
	DELL;// pause 100 ms 
	}// while 
}


void MenuProcess()
{
	switch (menu_pos) {
		// Main menu
		case 10:// Barrier
		LCD_Sense();
		break;
		
		case 11:
		SetSense();
		break;
		
		case 110:
		// Exit in Main menu
		menu_pos = 10;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		break;
		
		case 20:// GEB
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", mm1[1]);
		LCD_String(str01);
		LCD_SetPos(0, 1);
		sprintf(str01, "%lu", TIM3_GEB_value);
		LCD_String(str01);		
		break;
		
		case 21:// Setup GEB
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", str_Set_GEB);
		LCD_String(str01);
		break;
		
		case 210:// Ground balance coarse adjustment
		SetGEB();
		break;
		
		case 30:// Mask Ferrum
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", mm1[2]);
		LCD_String(str01);
		LCD_SetPos(0, 1);
		sprintf(str02, "Mode: %d", Mask_Ferrum_flag);
		LCD_String(str02);
		break;
		
		case 31:
		SetMaskFerrum();
		break;
		
		case 310:
		// Exit in Main menu
		menu_pos = 30;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		break;
		
		case 40:// Signal Buffering
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", mm1[3]);
		LCD_String(str01);
		LCD_SetPos(0, 1);
		sprintf(str02, "Mode: %d", SW_Signal_buff);
		LCD_String(str02);
		break;
		
		case 41:
		SetBufferingMode();
		break;
		
		case 410:
		// Exit in Main menu
		menu_pos = 40;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		break;
		
		case 50:// RX Sample bit Offset
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", mm1[4]);
		LCD_String(str01);
		LCD_SetPos(0, 1);
		sprintf(str02, "RX offset: %d", RX_Sample_bit_offset);
		LCD_String(str02);
		break;
		
		case 51:
		SetRXbit();
		break;
		
		case 510:
		// Exit in Main menu
		menu_pos = 50;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!! 
		break;
		
		case 60:// Coil balance
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", mm1[5]);
		LCD_String(str01);
		// balance millivolt
		Get_AMP_mv((float)Amp_B0_mV, (float)Amp_B1_mV, (float)Amp_B2_mV, (float)Amp_B3_mV);
		LCD_SetPos(9, 0);//LCD_SetPos(8, 0);
		sprintf(str01, "%ld", Amp_mv);
		LCD_String(str01);
		LCD_SetPos(14, 0);
		LCD_String("mV");
		// line 2
		LCD_SetPos(0, 1);
		sprintf(str02, "%ld", Amp_X);
		LCD_String(str02);
		LCD_SetPos(8, 1);
		sprintf(str02, "%ld", Amp_Y);
		LCD_String(str02);
		break;
		
		case 70:// TX Frequency
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", mm1[6]);
		LCD_String(str01);
		if (F_TX >= 10000) {
			LCD_SetPos(0, 1);
		} else{
			LCD_SetPos(1, 1);
		}		
		sprintf(str02, "%d", F_TX);
		LCD_String(str02);
		LCD_SetPos(6, 1);
		LCD_String("Hz");
		break;
		
		case 71:// Set Frequency
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", str_Set_freq);
		LCD_String(str01);
		break;
		
		case 710:// Setup Frequency Function
		SetFrequency();
		break;
		
		case 72:// Auto Search Frequency
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", str_Search_freq_01);
		LCD_String(str01);
		LCD_SetPos(0, 1);
		sprintf(str02, "%s", str_Search_freq_02);
		LCD_String(str02);
		break;
		
		case 720:// Auto Search Frequency Function
		LCD_Clear();// clr
		LCD_String("Search Start");		
		LL_mDelay(1000);
		// Call AutoSearch
		Search_Freq();
		// End AutoSearch
		LCD_SetPos(9, 1);
		LCD_String("OK");//LCD_String("OK -100");
		LL_mDelay(2000);
		// Exit in Main menu
		menu_pos = 70;
		p_menu_flag = 0;// clr Programm menu flag
		s_menu_flag = 0;// clr flag Sub menu
		m_menu_flag = 1;// set flag Main !!!
		//DELL;
		// Calibration
		Calib_auto = 0;// Auto Calibration OFF
		Calib_flag = 1;// Set flag - Get Calibration Samples
		break;
		
		case 80:// Target Speed
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", mm1[7]);
		LCD_String(str01);
		GetTargetSpeed();
		LCD_SetPos(0, 1);
		sprintf(str01, "%lu", (uint32_t)TargetSpeed);
		LCD_String(str01);
		LCD_SetPos(6, 1);
		LCD_String("Hz");
		break;
		
		case 81:// Setup Speed
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", str_Set_speed);
		LCD_String(str01);
		break;
		
		case 810:// Setup Speed
		SelectTargetSpeed();
		break;
		
		case 90:
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		LCD_String("FM Transmitter");
		LCD_SetPos(0, 1);
		if (KT0803L_ON_flag == 1) {
			sprintf(str02, "%.2f", KT0803L_Frequency);// extern float KT0803L_Frequency;
			LCD_String(str02);
		} else {
			sprintf(str02, "%s", str_OFF);
			LCD_String(str02);
		}
		break;
		
		case 91:// KT0803L Set Frequency - set channel
		LCD_Clear();// clr
		sprintf(str01, "%s", str_Set_freq);
		LCD_String(str01);
		LCD_SetPos(0, 1);
		LCD_String("kHz");
		break;
		
		case 910:// KT0803L Setup Frequency Function
		if (KT0803L_ON_flag == 1) {
			FM_SetFrequency();
		} else {
			LCD_Clear();// clr
			LCD_String("Chip disabled!!!");
			LCD_SetPos(0, 1);
			LCD_String("Select Chip Mode");
			DELL;
		}
		break;
		
		case 92:// KT0803L Power On / Off
		LCD_Clear();// clr
		LCD_String("Chip Mode");
		LCD_SetPos(0, 1);
		if (KT0803L_ON_flag == 1){
			sprintf(str02, "%s", str_ON);
			LCD_String(str02);
		} else {
			sprintf(str02, "%s", str_OFF);
			LCD_String(str02);
		}
		break;
		
		case 920:
		KT0803L_OnOff();
		break;
		
		/*
		case 100:// Ferrite calib
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", mm1[9]);
		LCD_String(str01);
		LCD_SetPos(0, 1);
		LCD_String("N/A");//LCD_String("N/A - Use Auto");
		break;
		*/
		
		/*
		case 110:// Backlight
		LCD_Clear();// clr
		//LCD_SetPos(0, 0);
		sprintf(str01, "%s", mm1[10]);
		LCD_String(str01);
		LCD_SetPos(0, 1);
		if (SW_Backlight == 0){
			sprintf(str02, "%s", str_OFF);
			LCD_String(str02);
		} else {
			sprintf(str02, "%s", str_ON);
			LCD_String(str02);
		}
		break;
		*/
		
		//default:
		//LCD_SetPos(0, 0);
		//LCD_String("default");
		//break;
	}	
}

void PollingButtons()
{
	// ENTER in Main menu
	if (Button_MENU() && (m_menu_flag == 0)){
		m_menu_flag = 1;
		menu_pos = 0;//10;
		sw_default = 0;// clr flag
		s_menu_flag = 0;
		// 
		LCD_Clear();// clr
		LCD_String("Main Menu");
		//LCD_SetPos(0, 0);
		//sprintf(str01, "%s", Enter_menu);
		//LCD_String(str01);
		LL_mDelay(1000);
		//
	}
	// EXIT of Main menu
	if (Button_MENU() && (m_menu_flag == 1)){
		m_menu_flag = 0;
		menu_pos = 0;
		sw_default = 1;// set flag
		// 
		//LCD_SetPos(0, 0);
		//sprintf(str01, "%s", Exit_menu);
		//LCD_String(str01);
		LCD_Clear();// clr
		LCD_String("Exit");
		LL_mDelay(500);//LL_mDelay(1000);
		Target_flag = 1;// update LCD
		//
	}
	// Main menu
	//if ((m_menu_flag == 1) && (s_menu_flag == 0)){// Flag Main menu
	if (m_menu_flag == 1){// Flag Main menu
		// Down
		//if (Button_DOWN() && (menu_pos < (uint16_t)MAIN_NENU_MAX) && (s_menu_flag == 0)){// if < 50
		if (Button_DOWN() && (s_menu_flag == 0)){
			menu_pos += (uint16_t)MAIN_NENU_STEP;// + 10
			if (menu_pos > (uint16_t)MAIN_NENU_MAX) { menu_pos = 0; } // v1.0.11 
			DELL;
		}
		// Up
		//if (Button_UP() && (menu_pos >= (uint16_t)MAIN_NENU_STEP_X2) && (s_menu_flag == 0)){// if >= 20
		if (Button_UP() && (s_menu_flag == 0)){
			menu_pos -= (uint16_t)MAIN_NENU_STEP;// - 10
			if (menu_pos > (uint16_t)MAIN_NENU_MAX) { menu_pos = (uint16_t)MAIN_NENU_MAX; } // v1.0.11 
			DELL;
		}
		// Right -->> Go to Sub menu
		if (Button_RIGHT() && (s_menu_flag == 0)){// if == 0
			menu_pos_old = menu_pos;// Save old position menu - 10, 20, 30, 40, 50.
			menu_pos += (uint16_t)SUB_MENU_STEP;// + 1
			s_menu_flag = 1;// set flag
			m_menu_flag = 0;// clr flag	!!!
			DELL;
		}
		// Left <<-- Go to Main menu
		//if (Button_LEFT() && (s_menu_flag == 1)){// if == 1 
			//menu_pos = menu_pos_old;// Set old position menu - 10, 20, 30, 40, 50.
			//s_menu_flag = 0;// clr flag
			//sub_menu_pos = 0;// clr position sub menu			
		//}	
	}
	// Sub menu
	if ((s_menu_flag == 1) && (m_menu_flag == 0)){// Flag Sub menu
		
		// Pause !!!
		//LL_mDelay(250);// 250 ms
		
		// Down
		if (Button_DOWN() && (menu_pos < (menu_pos_old + (uint16_t)SUB_MENU_NB))){// if < 13, if < 23, if < 33, if < 43, if < 53. 
			menu_pos += (uint16_t)SUB_MENU_STEP;// + 1
			DELL;
		}
		// Up
		if (Button_UP() && (menu_pos > (menu_pos_old + (uint16_t)SUB_MENU_STEP))){// if > 11, if > 21, if > 31, if > 41, if > 51. 
			menu_pos -= (uint16_t)SUB_MENU_STEP;// - 1
			DELL;
		}
		// Right -->> Function
		if (Button_RIGHT() && (p_menu_flag == 0)){// if == 0
			sub_menu_pos = menu_pos;// Save position sub menu - 11,12,13, 21,22,23, 31,32,33, 41,42,43, 51,52,53.
			menu_pos = menu_pos * 10;// = menu_pos * (uint16_t)SUB_MENU_PROG;// Mult 10
			p_menu_flag = 1;// SET Programm menu flag
			DELL;
		}
		// Left <<-- Go to Sub menu
		if (Button_LEFT() && (p_menu_flag == 1)){// if == 1
			// Do not divide by 10, but restore the old value.
			//menu_pos = sub_menu_pos;// Set position sub menu - 11,12,13, 21,22,23, 31,32,33, 41,42,43, 51,52,53.
			menu_pos = menu_pos / (uint16_t)SUB_MENU_PROG;// Divide 10
			p_menu_flag = 0;// CLR Programm menu flag \ CLR flag end function.
			s_menu_flag = 1;// set flag !
			DELL;
		}
		// Left <<-- Go to Main menu
		if (Button_LEFT() && (s_menu_flag == 1)){
			menu_pos = menu_pos_old;// Set old position menu - 10, 20, 30, 40, 50.
			s_menu_flag = 0;// clr flag sub menu
			sub_menu_pos = 0;// clr position sub menu
			m_menu_flag = 1;// set flag	!!!
			DELL;
		}		
	}

	// debug 
	/*
	if (1) {//if (0) {
		if (menu_pos >= 0 && menu_pos < 10) { LCD_SetPos(13, 1); LCD_String("  "); LCD_SetPos(15, 1); }
		if (menu_pos >= 10 && menu_pos < 100) { LCD_SetPos(13, 1); LCD_String(" "); LCD_SetPos(14, 1); }
		if (menu_pos >= 100 && menu_pos < 1000) { LCD_SetPos(13, 1); }
		char str01[6];
		sprintf(str01, "%d", menu_pos);
		//LCD_String("   \0");
		LCD_String(str01);
		//LL_mDelay(500);
	}
	*/
}



void MenuProcessProg(void)
{
	//MenuProcess();
	PollingButtons();
	
	if (sw_default == 0) {//if (sw_default == 0 && (menu_pos >= 10)) {
		// Menu Process
		MenuProcess();
		// Pause
		LL_mDelay(100);
		//LL_mDelay(200);
	}	
}




// end


/* USER CODE END 0 */


