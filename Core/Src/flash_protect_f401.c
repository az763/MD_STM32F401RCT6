/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : flash_protect_f401.c
  * @brief          : flash_protect program body
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
#include <stdio.h>
#include <string.h>
#include "flash_protect_f401.h"
//#include "stm32f4xx_ll_utils.h"// LL_mDelay()

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/*
volatile uint16_t *idBase0 = (uint16_t*)(UID_BASE);
volatile uint16_t *idBase1 = (uint16_t*)(UID_BASE + 0x02);
volatile uint32_t *idBase2 = (uint32_t*)(UID_BASE + 0x04);
volatile uint32_t *idBase3 = (uint32_t*)(UID_BASE + 0x08);

volatile uint16_t uidBase0 = 0;
volatile uint16_t uidBase1 = 0;
volatile uint32_t uidBase2 = 0;
volatile uint32_t uidBase3 = 0;

// Flags UID
extern volatile uint8_t UID_0_OK;
extern volatile uint8_t UID_1_OK;
extern volatile uint8_t UID_2_OK;
extern volatile uint8_t UID_3_OK;
*/

/* USER CODE END PD */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//------------------------------------------------
/*
void Read_Device_ID(void)
{
	//char buffer[64] = {0,};
	//sprintf(buffer, "\n %X-%X-%lX-%lX\n", *idBase0, *idBase1, *idBase2, *idBase3);
	//printf("%s \n", buffer);
	// copy uid
	uidBase0 = *(uint16_t *)idBase0;
	uidBase1 = *(uint16_t *)idBase1;
	uidBase2 = *(uint32_t *)idBase2;
	uidBase3 = *(uint32_t *)idBase3;
	// Send UID to UART
	printf("\n\n %X-%X-%lX-%lX\n\n", uidBase0, uidBase1, uidBase2, uidBase3);
}

void CMP_UID_0(void)
{
	if(uidBase0 == (uint16_t)UID_0){
		UID_0_OK = 1;// UID True
	}
	else{
		UID_0_OK = 0;// UID False
	}	
}

void CMP_UID_1(void)
{
	if(uidBase1 == (uint16_t)UID_1){
		UID_1_OK = 1;// UID True
	}
	else{
		UID_1_OK = 0;// UID False
	}	
}


void CMP_UID_2(void)
{
	if(uidBase2 == (uint32_t)UID_2){
		UID_2_OK = 1;// UID True
	}
	else{
		UID_2_OK = 0;// UID False
	}	
}



void CMP_UID_3(void)
{
	if(uidBase3 == (uint32_t)UID_3){
		UID_3_OK = 1;// UID True
	}
	else{
		UID_3_OK = 0;// UID False
	}	
}
*/

#include "stm32f4xx.h"


uint32_t flash_ready(void)
{
	return (!(FLASH->SR & FLASH_SR_BSY));
}


// RDP - ReaDout Protection. Write Protection Level 1.
void Flash_Write_RDP_Level1(void)
{
	volatile uint8_t OB_RDP_Level;
	
	OB_RDP_Level = *(__IO uint8_t*)(OPTCR_BYTE1_ADDRESS);
	
	printf("OB_RDP_Level: %02X\n", OB_RDP_Level);
	
	if (OB_RDP_Level == (uint8_t)OB_RDP_Level_0) {
		
		printf("RDP Disabled\n");
		
		// FLASH_Unlock()
		//if((FLASH->CR & FLASH_CR_LOCK) != RESET)
		//{
		//	// Authorize the FLASH Registers access 
		//	FLASH->KEYR = FLASH_KEY1;// 0x45670123UL
		//	FLASH->KEYR = FLASH_KEY2;// 0xCDEF89ABUL
		//}
		//while (!flash_ready());// Wait
		
		// OB Unlock - Enable the Flash option control register access
		if((FLASH->OPTCR & FLASH_OPTCR_OPTLOCK) != RESET)
		{
			// Authorizes the Option Byte register programming 
			FLASH->OPTKEYR = FLASH_OPT_KEY1;// 0x08192A3BU
			FLASH->OPTKEYR = FLASH_OPT_KEY2;// 0x4C5D6E7FU
		}
		while (!flash_ready());// Wait
		// Wait for last operation to be completed
		//while ((FLASH->SR & FLASH_SR_BSY) != 0 );// Wait
		
		// Write DRP Level 1
		// FLASH_OB_RDPConfig()
		uint8_t OB_RDP = OB_RDP_Level_1;// = 0x55;
		
		// write byte
		*(__IO uint8_t*)OPTCR_BYTE1_ADDRESS = OB_RDP;
		
		// Once all needed Option Bytes to be programmed are correctly written, 
        // call the FLASH_OB_Launch() function to launch the Option Bytes programming process.
		// 
		// FLASH_OB_Launch() - Start the Option Bytes programming process.
		
		while (!flash_ready());// Wait
		
		/* Set the OPTSTRT bit in OPTCR register */
		*(__IO uint8_t *)OPTCR_BYTE0_ADDRESS |= FLASH_OPTCR_OPTSTRT;
		
		while (!flash_ready());// Wait
		
		// End the Option Bytes programming process.
		
		printf("SET OB_RDP_Level_1\n");
		
		// FLASH_OB_Lock()
		/* Set the OPTLOCK Bit to lock the FLASH Option Byte Registers access */
		FLASH->OPTCR |= FLASH_OPTCR_OPTLOCK;
		
		// FLASH_Lock()
		/* Set the LOCK Bit to lock the FLASH Registers access */
		//FLASH->CR |= FLASH_CR_LOCK;
	} 
	else {
		printf("RDP Enabled\n");
	}
	
}

//------------------------------------------------

/* USER CODE END 0 */
