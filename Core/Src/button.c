/*
 * button.c
 *
 *  Created on: Jan 5, 2023
 *      Author: as
 */
 
 /* Includes ------------------------------------------------------------------*/
#include "button.h"

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define Button_nb 5 // x 10 ms = 50 ms
volatile uint8_t button_cnt;
volatile uint8_t i;
/* USER CODE END PD */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// SW1
uint8_t Button_OK()
{
	if (BTN_OK) {// if 1 - not pressed
		return 0;// exit
	} else {// if 0 - pressed
		// clear counter
		button_cnt = 0;
		// cycle
		for (i = 0; i < Button_nb; i++) {
			LL_mDelay(10);// pause 10 ms.
			if (BTN_OK) {// if 1 - not pressed
				//button_cnt = 0;
				return 0;// exit
			} else {// if 0 - pressed
				button_cnt++;
			}			
		}
		if (button_cnt == (uint8_t)Button_nb) return 1;// PRESSED BUTTON
	}	
	return 0;// exit
}

// SW2
uint8_t Button_RIGHT()
{
	if (BTN_RIGHT) {// if 1 - not pressed
		return 0;// exit
	} else {// if 0 - pressed
		// clear counter
		button_cnt = 0;
		// cycle
		for (i = 0; i < Button_nb; i++) {
			LL_mDelay(10);// pause 10 ms.
			if (BTN_RIGHT) {// if 1 - not pressed
				//button_cnt = 0;
				return 0;// exit
			} else {// if 0 - pressed
				button_cnt++;
			}			
		}
		if (button_cnt == (uint8_t)Button_nb) return 1;// PRESSED BUTTON
	}	
	return 0;// exit
}

// SW3
uint8_t Button_DOWN()
{
	if (BTN_DOWN) {// if 1 - not pressed
		return 0;// exit
	} else {// if 0 - pressed
		// clear counter
		button_cnt = 0;
		// cycle
		for (i = 0; i < Button_nb; i++) {
			LL_mDelay(10);// pause 10 ms.
			if (BTN_DOWN) {// if 1 - not pressed
				//button_cnt = 0;
				return 0;// exit
			} else {// if 0 - pressed
				button_cnt++;
			}			
		}
		if (button_cnt == (uint8_t)Button_nb) return 1;// PRESSED BUTTON
	}	
	return 0;// exit
}

// SW4
uint8_t Button_UP()
{
	if (BTN_UP) {// if 1 - not pressed
		return 0;// exit
	} else {// if 0 - pressed
		// clear counter
		button_cnt = 0;
		// cycle
		for (i = 0; i < Button_nb; i++) {
			LL_mDelay(10);// pause 10 ms.
			if (BTN_UP) {// if 1 - not pressed
				//button_cnt = 0;
				return 0;// exit
			} else {// if 0 - pressed
				button_cnt++;
			}			
		}
		if (button_cnt == (uint8_t)Button_nb) return 1;// PRESSED BUTTON
	}	
	return 0;// exit
}

// SW5
uint8_t Button_LEFT()
{
	if (BTN_LEFT) {// if 1 - not pressed
		return 0;// exit
	} else {// if 0 - pressed
		// clear counter
		button_cnt = 0;
		// cycle
		for (i = 0; i < Button_nb; i++) {
			LL_mDelay(10);// pause 10 ms.
			if (BTN_LEFT) {// if 1 - not pressed
				//button_cnt = 0;
				return 0;// exit
			} else {// if 0 - pressed
				button_cnt++;
			}			
		}
		if (button_cnt == (uint8_t)Button_nb) return 1;// PRESSED BUTTON
	}	
	return 0;// exit
}

// SW6
uint8_t Button_MENU()
{
	if (BTN_MENU) {// if 1 - not pressed
		return 0;// exit
	} else {// if 0 - pressed
		// clear counter
		button_cnt = 0;
		// cycle
		for (i = 0; i < Button_nb; i++) {
			LL_mDelay(10);// pause 10 ms.
			if (BTN_MENU) {// if 1 - not pressed
				//button_cnt = 0;
				return 0;// exit
			} else {// if 0 - pressed
				button_cnt++;
			}			
		}
		if (button_cnt == (uint8_t)Button_nb) return 1;// PRESSED BUTTON
	}	
	return 0;// exit
}
// end


/* USER CODE END 0 */


