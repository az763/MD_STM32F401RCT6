/* USER CODE BEGIN Header */
// STM Урок 163. LL. USART. Interrupt. https://narodstream.ru/stm-urok-163-ll-usart-interrupt/
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile char rx_str[32];
volatile char tx_str[32];
volatile uint8_t fl = 0;
volatile uint8_t dt1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//--------------------------------------------------------
void USART_TX(uint8_t* dt, uint16_t sz)
{
  uint16_t ind = 0;
  while (ind<sz)
  {
    while (!LL_USART_IsActiveFlag_TXE(USART1)) {}
    LL_USART_TransmitData8(USART1,*(uint8_t*)(dt+ind));
    ind++;
  }
}
//--------------------------------------------------------
void USART_RX_TX_Str(uint8_t* tx_dt, uint8_t* rx_dt)//uint16_t USART_RX_TX_Str(uint8_t* tx_dt, uint8_t* rx_dt)
{
  uint16_t ind = 0;
  while (!fl) {}
  fl=0;
  rx_dt[ind] = dt1;
  while (!LL_USART_IsActiveFlag_TXE(USART1)) {}
  LL_USART_TransmitData8(USART1,*(uint8_t*)(tx_dt+ind));
  while(rx_dt[ind])
  {
    ind++;
    while (!fl) {}
    fl=0;
    rx_dt[ind] = dt1;
    while (!LL_USART_IsActiveFlag_TXE(USART1)) {}
    LL_USART_TransmitData8(USART1,*(uint8_t*)(tx_dt+ind));
  }
  //return ind;
}
//--------------------------------------------------------
/* USER CODE END 0 */


#ifdef __cplusplus
}
#endif

#endif /* __USART_H */

// !!! add in main.c

//#include "usart.h"

// Enable Interrupt

//LL_USART_EnableIT_RXNE(USART1);
//LL_USART_EnableIT_ERROR(USART1); 

/*
void USART1_RX_Callback(void)
{
	dt1 = LL_USART_ReceiveData8(USART1);
	fl = 1;
}
*/

// !!! add in stm32f1xx_it.c

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
//void USART1_RX_Callback(void);
/* USER CODE END PFP */

// This function handles USART1 global interrupt.
/*
void USART1_IRQHandler(void)
{
  // USER CODE BEGIN USART1_IRQn 0
  
  if(LL_USART_IsActiveFlag_RXNE(USART1) && LL_USART_IsEnabledIT_RXNE(USART1))
  {
    USART1_RX_Callback();
  }
  else
  {
    if(LL_USART_IsActiveFlag_ORE(USART1))
    {
      (void) USART1->DR;
    }
    else if(LL_USART_IsActiveFlag_FE(USART1))
    {
      (void) USART1->DR;
    }
    else if(LL_USART_IsActiveFlag_NE(USART1))
    {
      (void) USART1->DR;
    }
  }
  
  // USER CODE END USART1_IRQn 0
  // USER CODE BEGIN USART1_IRQn 1

  // USER CODE END USART1_IRQn 1
}
*/

























