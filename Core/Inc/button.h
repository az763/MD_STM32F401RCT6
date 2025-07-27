/*
 * button.h
 *
 *  Created on: Jan 5, 2023
 *      Author: as
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Private defines */

// Button
//#define BTN_OK    (GPIOB->IDR & (1 << 9))// PB9 BTN0 (46) pcb SW1 - OK    - "Enter / OK / Ground balance"
//#define BTN_RIGHT (GPIOB->IDR & (1 << 8))// PB8 BTN1 (45) pcb SW2 - RIGHT - "Right (+) / PinPointer"
//#define BTN_DOWN  (GPIOB->IDR & (1 << 7))// PB7 BTN2 (43) pcb SW3 - DOWN  - "Down / Barrier- / Autotune"
//#define BTN_UP    (GPIOB->IDR & (1 << 6))// PB6 BTN3 (42) pcb SW4 - UP    - "Up / Barrier+ / Autotune"
//#define BTN_LEFT  (GPIOB->IDR & (1 << 5))// PB5 BTN4 (41) pcb SW5 - LEFT  - "Left (-) / Backlight"
//#define BTN_MENU  (GPIOB->IDR & (1 << 4))// PB4 BTN5 (40) pcb SW6 - MENU  - "Menu / Esc"
// LL
#define BTN_OK    LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_9)// PB9 BTN0 (46) pcb SW1 - OK    - "Enter / OK / Ground balance"
#define BTN_RIGHT LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_8)// PB8 BTN1 (45) pcb SW2 - RIGHT - "Right (+) / PinPointer"
#define BTN_DOWN  LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_7)// PB7 BTN2 (43) pcb SW3 - DOWN  - "Down / Barrier- / Autotune"
#define BTN_UP    LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_6)// PB6 BTN3 (42) pcb SW4 - UP    - "Up / Barrier+ / Autotune"
#define BTN_LEFT  LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_5)// PB5 BTN4 (41) pcb SW5 - LEFT  - "Left (-) / Backlight"
#define BTN_MENU  LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_4)// PB4 BTN5 (40) pcb SW6 - MENU  - "Menu / Esc"
// :)
/* Exported functions prototypes ---------------------------------------------*/
uint8_t Button_OK();
uint8_t Button_RIGHT();
uint8_t Button_DOWN();
uint8_t Button_UP();
uint8_t Button_LEFT();
uint8_t Button_MENU();

/* USER CODE END Private defines */

#endif /* INC_BUTTON_H_ */
