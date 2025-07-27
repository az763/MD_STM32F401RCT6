/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : flash_protect_f401.h
  * @brief          : Header for flash_protect_f401.c file.
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
#ifndef INC_FLASH_PROTECT_F401_H_
#define INC_FLASH_PROTECT_F401_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f401xc.h"


/* USER CODE BEGIN Private defines */

//#define UID_BASE 0x1FFF7A10UL           /*!< Unique device ID register base address */

//#define UID_PROTECTION 1 // 0 - Disable, 1 - Enable.

// RE-WRITE UID TARGET CPU !!!
// 
//#define UID_0 0x0A29
//#define UID_1 0x1E00
//#define UID_2 0x30301671
//#define UID_3 0x32303030
// 
// end RE-WRITE

#define OPTCR_BYTE0_ADDRESS         ((uint32_t)0x40023C14) // redefined ?
#define OPTCR_BYTE1_ADDRESS         ((uint32_t)0x40023C15) // redefined ?

#define OB_RDP_Level_0   ((uint8_t)0xAA)
#define OB_RDP_Level_1   ((uint8_t)0x55)
//#define OB_RDP_Level_2   ((uint8_t)0xCC) 
/*!< Warning: When enabling read protection level 2 it's no more possible to go back to level 1 or 0 */

//#define RDP_KEY                  ((uint16_t)0x00A5)
//#define FLASH_KEY1               0x45670123U
//#define FLASH_KEY2               0xCDEF89ABU
#define FLASH_OPT_KEY1           0x08192A3BU
#define FLASH_OPT_KEY2           0x4C5D6E7FU

//#define FLASH_PSIZE_BYTE           0x00000000U
//#define FLASH_PSIZE_HALF_WORD      0x00000100U
//#define FLASH_PSIZE_WORD           0x00000200U
//#define FLASH_PSIZE_DOUBLE_WORD    0x00000300U
//#define CR_PSIZE_MASK              0xFFFFFCFFU

/* USER CODE END Private defines */


/* Exported functions prototypes ---------------------------------------------*/
void         FLASH_Unlock(void);
void         FLASH_Lock(void);
void         FLASH_OB_Unlock(void);
void         FLASH_OB_Lock(void);

//void Read_Device_ID(void);
//void CMP_UID_0(void);
//void CMP_UID_1(void);
//void CMP_UID_2(void);
//void CMP_UID_3(void);

uint32_t flash_ready(void);
void Flash_Write_RDP_Level1(void);



#ifdef __cplusplus
}
#endif

#endif /* INC_FLASH_PROTECT_F401_H_ */
