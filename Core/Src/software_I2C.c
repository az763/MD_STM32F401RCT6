/*
 * software_I2C.c
 *
 *  This source code from website forum.cxem.net 
 *  	URL: https://forum.cxem.net/index.php?/topic/203681-%D0%BF%D1%80%D0%BE%D0%B3%D1%80%D0%B0%D0%BC%D0%BC%D0%BD%D1%8B%D0%B9-i2c-%D0%B4%D0%BB%D1%8F-stm32/ 
 *
 *  	Topic: Software I2C for STM32.
 *  	Posted on: August 13, 2018.
 *  	Author: G1KuL1N.
 *
 *  File created on: Mar 2, 2024.
 *      This source code modified by: Aleksandr Z.
 *  
 */

/* Includes ------------------------------------------------------------------*/
#include "software_I2C.h"


/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
volatile uint8_t i2c_frame_error=0; 

/* USER CODE END PD */


/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


//-----------------------------------------------------------
__STATIC_INLINE void Delay_us(__IO uint32_t us) // __IO - volatile
{
	us *= (SystemCoreClock / 1000000) / 5;
	while(us--);
}

//----------------------------------------------------
void SCL_in (void) //функция отпускания SCL в 1, порт на вход (необходимо установить используемый порт) 
{
	//GPIO_InitTypeDef GPIO_InitStruct;
	//GPIO_InitStruct.Pin = GPIO_PIN_15;// PC15 - SCL 
	//GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	//HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	// LL
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LL_GPIO_PIN_15;// PC15 - SCL 
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	//GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

//----------------------------------------------------
void SCL_out (void) //функция притягивания SCL в 0 (необходимо установить используемый порт) 
{
	//GPIO_InitTypeDef GPIO_InitStruct;
	//GPIO_InitStruct.Pin = GPIO_PIN_15;// PC15 - SCL 
	//GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	//GPIO_InitStruct.Pull = GPIO_NOPULL;
	//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	//HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	//SCL_O;
	
	// LL
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LL_GPIO_PIN_15;// PC15 - SCL 
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	//GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	SCL_O;
}

//----------------------------------------------------
void SDA_in (void) //функция отпускания SDA в 1, порт на вход (необходимо установить используемый порт) 
{
	//GPIO_InitTypeDef GPIO_InitStruct;
	//GPIO_InitStruct.Pin = GPIO_PIN_14;// PC14 - SDA 
	//GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	//HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	// LL
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LL_GPIO_PIN_14;// PC14 - SDA 
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	//GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

//----------------------------------------------------
void SDA_out (void) //функция притягивания SDA в 0 (необходимо установить используемый порт) 
{
	//GPIO_InitTypeDef GPIO_InitStruct;
	//GPIO_InitStruct.Pin = GPIO_PIN_14;// PC14 - SDA 
	//GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	//GPIO_InitStruct.Pull = GPIO_NOPULL;
	//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	//HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	//SDA_O;
	
	// LL
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LL_GPIO_PIN_14;// PC14 - SDA 
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	//GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	SDA_O;
}

//----------------------------------------------------
void i2c_stop_cond (void)  // функция генерации условия стоп 
{
	uint16_t SCL, SDA;
	SCL_out(); // притянуть SCL (лог.0)
	Delay_us(10);
	SDA_out(); // притянуть SDA (лог.0)
	Delay_us(10);

	SCL_in(); // отпустить SCL (лог.1)
	Delay_us(10);
	SDA_in(); // отпустить SDA (лог.1)
	Delay_us(10);

	// проверка фрейм-ошибки
	i2c_frame_error=0;		// сброс счётчика фрейм-ошибок
	SCL=SCL_I;
	SDA=SDA_I;
	if (SCL == 0) i2c_frame_error++;   // проберяем, чтобы на ноге SCL была лог.1, иначе выдаём ошибку фрейма
	if (SDA == 0) i2c_frame_error++;   // проберяем, чтобы на ноге SDA была лог.1, иначе выдаём ошибку фрейма
	Delay_us(40);
}

void i2c_init (void) // функция инициализации шины
{
	i2c_stop_cond();   // стоп шины
	i2c_stop_cond();   // стоп шины
}

//----------------------------------------------------
void i2c_start_cond (void)  // функция генерации условия старт
{
	// !!! add this lines - copy restart cond. 
	SDA_in(); // отпустить SDA (лог.1)
	Delay_us(10);
	SCL_in(); // отпустить SCL (лог.1)
	Delay_us(10);
	//
	SDA_out(); // притянуть SDA (лог.0)
	Delay_us(10);
	SCL_out(); // притянуть SCL (лог.0)
	Delay_us(10);
}

//----------------------------------------------------
void i2c_restart_cond (void)   // функция генерации условия рестарт
{
	SDA_in(); // отпустить SDA (лог.1)
	Delay_us(10);
	SCL_in(); // отпустить SCL (лог.1)
	Delay_us(10);
	SDA_out(); // притянуть SDA (лог.0)
	Delay_us(10);
	SCL_out(); // притянуть SCL (лог.0)
	Delay_us(10);
}

//----------------------------------------------------
uint8_t i2c_send_byte (uint8_t data)  // функция  отправки байта  
{   
	uint8_t i;
	uint8_t ack=1;// АСК, если АСК=1 – произошла ошибка
	uint16_t SDA;   
	for (i=0;i<8;i++)
	{
		if (data & 0x80) 
		{
			SDA_in(); // лог.1
		}
		else 
		{
			SDA_out(); // Выставить бит на SDA (лог.0)
		}
		Delay_us(10);
		SCL_in();   // Записать его импульсом на SCL       // отпустить SCL (лог.1)
		Delay_us(10);
		SCL_out(); // притянуть SCL (лог.0)
		data<<=1; // сдвигаем на 1 бит влево

	}
	SDA_in(); // отпустить SDA (лог.1), чтобы ведомое устройство смогло сгенерировать ACK
	Delay_us(10);
	SCL_in(); // отпустить SCL (лог.1), чтобы ведомое устройство передало ACK
	Delay_us(10);
	SDA=SDA_I;
	if (SDA==0x00) ack=1; else ack=0;    // Считать ACK

	SCL_out(); // притянуть SCL (лог.0)  // приём ACK завершён

	return ack; // вернуть ACK (0) или NACK (1)   

}

//----------------------------------------------------
uint8_t i2c_get_byte (uint8_t last_byte) // функция принятия байта
{
	uint8_t i, res=0;
	uint16_t SDA;
	SDA_in(); // отпустить SDA (лог.1)

	for (i=0;i<8;i++)
	{
		res<<=1;
		SCL_in(); // отпустить SCL (лог.1)      //Импульс на SCL
		Delay_us(10);
		SDA_in();
		SDA=SDA_I;
		if (SDA==1) res=res|0x01; // Чтение SDA в переменную  Если SDA=1 то записываем 1
		SCL_out(); // притянуть SCL (лог.0)
		Delay_us(10);
	}

	if (last_byte==0){ SDA_out();} // притянуть SDA (лог.0)     // Подтверждение, ACK, будем считывать ещё один байт
	else {SDA_in();} // отпустить SDA (лог.1)                 // Без подтверждения, NACK, это последний считанный байт
	Delay_us(10);
	SCL_in(); // отпустить SCL (лог.1)
	Delay_us(10);
	SCL_out(); // притянуть SCL (лог.0)
	Delay_us(10);
	SDA_in(); // отпустить SDA (лог.1)

	return res; // вернуть считанное значение
}


//------------------------------------------------
// This source code modified by: Aleksandr Z.
//------------------------------------------------
/*
void EEPROM_Write16(uint16_t addr, uint16_t data)
{
	i2c_start_cond();
	i2c_send_byte((SLAVE_OWN_ADDRESS | I2C_REQUEST_WRITE));// send 0xA0
	//i2c_send_byte((addr & 0xFF00) >> 8);// AT24C16 addressing 1 byte.
	i2c_send_byte(addr & 0x00FF);
	i2c_send_byte((data & 0xFF00) >> 8);  
	i2c_send_byte(data & 0x00FF);
	i2c_stop_cond();
}

//------------------------------------------------
uint16_t EEPROM_Read16(uint16_t addr)
{
	uint16_t data_out;
	uint8_t buffer[2];
	i2c_start_cond();
	i2c_send_byte((SLAVE_OWN_ADDRESS | I2C_REQUEST_WRITE));// send 0xA0
	//i2c_send_byte((addr & 0xFF00) >> 8);// AT24C16 addressing 1 byte.
	i2c_send_byte(addr & 0x00FF);
	i2c_restart_cond ();
	i2c_send_byte((SLAVE_OWN_ADDRESS | I2C_REQUEST_READ));// send 0xA1
	buffer[0] = i2c_get_byte(0x00);
	buffer[1] = i2c_get_byte(0x01);
	i2c_stop_cond();
	data_out = (uint16_t)(buffer[0] << 8) + (uint16_t)(buffer[1]);
	return data_out;
}
*/
//------------------------------------------------
//#define EEP_SIZE 1024 // 24C08
#define EEP_SIZE 2048 // 24C16
#define ADDR_H_MASK ((EEP_SIZE - 1) >> 8)

void EEPROM_Write16(uint16_t addr, uint16_t data)
{
	uint8_t addr_h = addr >> 8;
	
	addr_h &= ADDR_H_MASK;// mask 2 kByte if eeprom 24C16
	addr_h <<= 1;// shift 1
	addr_h |= SLAVE_OWN_ADDRESS;// |= 0xA0
	addr_h |= I2C_REQUEST_WRITE;// |= 0x00
	// start
	i2c_start_cond();
	// send address
	i2c_send_byte(addr_h);	
	i2c_send_byte(addr & 0xFF);
	// send data
	i2c_send_byte(data >> 8);  
	i2c_send_byte(data & 0xFF);
	// stop
	i2c_stop_cond();
}

//------------------------------------------------
uint16_t EEPROM_Read16(uint16_t addr)
{
	uint16_t data_out;
	uint8_t buffer[2];
	uint8_t addr_h = addr >> 8;
	
	addr_h &= ADDR_H_MASK;// mask 2 kByte if eeprom 24C16
	addr_h <<= 1;// shift 1
	// start
	i2c_start_cond();
	// send address + request write
	i2c_send_byte((addr_h | SLAVE_OWN_ADDRESS | I2C_REQUEST_WRITE));
	i2c_send_byte(addr & 0xFF);
	// restart
	i2c_restart_cond ();
	// send request read
	i2c_send_byte((SLAVE_OWN_ADDRESS | I2C_REQUEST_READ));
	// read
	buffer[0] = i2c_get_byte(0x00);
	buffer[1] = i2c_get_byte(0x01);
	// stop
	i2c_stop_cond();
	
	data_out = (uint16_t)(buffer[0] << 8) + (uint16_t)(buffer[1]);
	return data_out;
}

//------------------------------------------------

/* USER CODE END 0 */

