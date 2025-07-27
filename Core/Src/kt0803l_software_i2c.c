/*
 * kt0803l_software_i2c.c
 *
 *  File created on: Sep. 26, 2024.
 *      This source code modified by: Aleksandr Z.
 *  
 */

/* Includes ------------------------------------------------------------------*/
#include "kt0803l_software_i2c.h"


/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
volatile uint8_t kt0803l_i2c_frame_error=0; 
volatile uint8_t error_nb = 0;

/* USER CODE END PD */


/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


//-----------------------------------------------------------
__STATIC_INLINE void KT0803L_Delay_us(__IO uint32_t us) // __IO - volatile
{
	us *= (SystemCoreClock / 1000000) / 5;
	while(us--);
}

//----------------------------------------------------
void KT0803L_SCL_in(void) //функция отпускания SCL в 1, порт на вход (необходимо установить используемый порт) 
{
	// HAL 
	//GPIO_InitTypeDef GPIO_InitStruct = {0};
	//GPIO_InitStruct.Pin = GPIO_PIN_4;// PA4 - SCL 
	//GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	//HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// LL 
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LL_GPIO_PIN_4;// PA4 - SCL 
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	//GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

//----------------------------------------------------
void KT0803L_SCL_out(void) //функция притягивания SCL в 0 (необходимо установить используемый порт) 
{
	// HAL 
	//GPIO_InitTypeDef GPIO_InitStruct = {0};
	//GPIO_InitStruct.Pin = GPIO_PIN_4;// PA4 - SCL 
	//GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	//GPIO_InitStruct.Pull = GPIO_NOPULL;
	//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	//HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	//KT0803L_SCL_O;
	
	// LL 
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LL_GPIO_PIN_4;// PA4 - SCL 
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	//GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	KT0803L_SCL_O;
}

//----------------------------------------------------
void KT0803L_SDA_in(void) //функция отпускания SDA в 1, порт на вход (необходимо установить используемый порт) 
{
	// HAL 
	//GPIO_InitTypeDef GPIO_InitStruct = {0};
	//GPIO_InitStruct.Pin = GPIO_PIN_5;// PA5 - SDA 
	//GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	//HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// LL 
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LL_GPIO_PIN_5;// PA5 - SDA 
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	//GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

//----------------------------------------------------
void KT0803L_SDA_out(void) //функция притягивания SDA в 0 (необходимо установить используемый порт) 
{
	// HAL 
	//GPIO_InitTypeDef GPIO_InitStruct = {0};
	//GPIO_InitStruct.Pin = GPIO_PIN_5;// PA5 - SDA 
	//GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	//GPIO_InitStruct.Pull = GPIO_NOPULL;
	//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	//HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	//KT0803L_SDA_O;
	
	// LL 
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LL_GPIO_PIN_5;// PA5 - SDA 
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	//GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	KT0803L_SDA_O;
}

//----------------------------------------------------
void KT0803L_i2c_stop_cond(void)  // функция генерации условия стоп 
{
	uint16_t SCL, SDA;
	KT0803L_SCL_out(); // притянуть SCL (лог.0)
	KT0803L_Delay_us(DELAY_TIME_US);
	KT0803L_SDA_out(); // притянуть SDA (лог.0)
	KT0803L_Delay_us(DELAY_TIME_US);

	KT0803L_SCL_in(); // отпустить SCL (лог.1)
	KT0803L_Delay_us(DELAY_TIME_US);
	KT0803L_SDA_in(); // отпустить SDA (лог.1)
	KT0803L_Delay_us(DELAY_TIME_US);

	// проверка фрейм-ошибки
	kt0803l_i2c_frame_error=0;		// сброс счётчика фрейм-ошибок
	SCL=KT0803L_SCL_I;
	SDA=KT0803L_SDA_I;
	if (SCL == 0) kt0803l_i2c_frame_error++;   // проберяем, чтобы на ноге SCL была лог.1, иначе выдаём ошибку фрейма
	if (SDA == 0) kt0803l_i2c_frame_error++;   // проберяем, чтобы на ноге SDA была лог.1, иначе выдаём ошибку фрейма
	//KT0803L_Delay_us(40);// old
	KT0803L_Delay_us(DELAY_TIME_US * 4);
	//KT0803L_Delay_us(DELAY_TIME_US);
}

void KT0803L_i2c_init(void) // функция инициализации шины
{
	KT0803L_i2c_stop_cond();   // стоп шины 
	KT0803L_i2c_stop_cond();   // стоп шины 
}

//----------------------------------------------------
void KT0803L_i2c_start_cond(void)  // функция генерации условия старт
{
	KT0803L_SDA_in();// added the line !!!
	KT0803L_Delay_us(DELAY_TIME_US);// added the line !!!
	KT0803L_SCL_in();// added the line !!!
	KT0803L_Delay_us(DELAY_TIME_US);// added the line !!!
	
	KT0803L_SDA_out(); // притянуть SDA (лог.0)
	KT0803L_Delay_us(DELAY_TIME_US);
	KT0803L_SCL_out(); // притянуть SCL (лог.0)
	KT0803L_Delay_us(DELAY_TIME_US);
}

//----------------------------------------------------
void KT0803L_i2c_restart_cond(void)   // функция генерации условия рестарт
{
	KT0803L_SDA_in(); // отпустить SDA (лог.1)
	KT0803L_Delay_us(DELAY_TIME_US);
	KT0803L_SCL_in(); // отпустить SCL (лог.1)
	KT0803L_Delay_us(DELAY_TIME_US);
	KT0803L_SDA_out(); // притянуть SDA (лог.0)
	KT0803L_Delay_us(DELAY_TIME_US);
	KT0803L_SCL_out(); // притянуть SCL (лог.0)
	KT0803L_Delay_us(DELAY_TIME_US);
}

//----------------------------------------------------
uint8_t KT0803L_i2c_send_byte(uint8_t data)  // функция  отправки байта  
{   
	uint8_t i;
	uint8_t ack=1;// АСК, если АСК=1 – произошла ошибка
	uint16_t SDA;   
	for (i=0;i<8;i++)
	{
		if (data & 0x80) 
		{
			KT0803L_SDA_in(); // лог.1
		}
		else 
		{
			KT0803L_SDA_out(); // Выставить бит на SDA (лог.0)
		}
		KT0803L_Delay_us(DELAY_TIME_US);
		KT0803L_SCL_in();   // Записать его импульсом на SCL       // отпустить SCL (лог.1)
		KT0803L_Delay_us(DELAY_TIME_US);
		KT0803L_SCL_out(); // притянуть SCL (лог.0)
		data<<=1; // сдвигаем на 1 бит влево

	}
	KT0803L_SDA_in(); // отпустить SDA (лог.1), чтобы ведомое устройство смогло сгенерировать ACK
	KT0803L_Delay_us(DELAY_TIME_US);
	KT0803L_SCL_in(); // отпустить SCL (лог.1), чтобы ведомое устройство передало ACK
	KT0803L_Delay_us(DELAY_TIME_US);
	SDA=KT0803L_SDA_I;
	if (SDA==0x00) ack=1; else ack=0;    // Считать ACK

	KT0803L_SCL_out(); // притянуть SCL (лог.0)  // приём ACK завершён

	return ack; // вернуть ACK (0) или NACK (1)   

}

//----------------------------------------------------
uint8_t KT0803L_i2c_get_byte(uint8_t last_byte) // функция принятия байта
{
	uint8_t i, res=0;
	uint16_t SDA;
	KT0803L_SDA_in(); // отпустить SDA (лог.1)

	for (i=0;i<8;i++)
	{
		res<<=1;
		KT0803L_SCL_in(); // отпустить SCL (лог.1)      //Импульс на SCL
		KT0803L_Delay_us(DELAY_TIME_US);
		KT0803L_SDA_in();
		SDA=KT0803L_SDA_I;
		if (SDA==1) res=res|0x01; // Чтение SDA в переменную  Если SDA=1 то записываем 1
		KT0803L_SCL_out(); // притянуть SCL (лог.0)
		KT0803L_Delay_us(DELAY_TIME_US);
	}

	if (last_byte==0){ KT0803L_SDA_out();} // притянуть SDA (лог.0)     // Подтверждение, ACK, будем считывать ещё один байт
	else {KT0803L_SDA_in();} // отпустить SDA (лог.1)                 // Без подтверждения, NACK, это последний считанный байт
	KT0803L_Delay_us(DELAY_TIME_US);
	KT0803L_SCL_in(); // отпустить SCL (лог.1)
	KT0803L_Delay_us(DELAY_TIME_US);
	KT0803L_SCL_out(); // притянуть SCL (лог.0)
	KT0803L_Delay_us(DELAY_TIME_US);
	KT0803L_SDA_in(); // отпустить SDA (лог.1)

	return res; // вернуть считанное значение
}


//------------------------------------------------

void KT0803L_WriteRegister(uint8_t reg_address, uint8_t value)
{
	uint8_t ack = 0;// 0 - error
	// Send START condition 
	KT0803L_i2c_start_cond();
	// 1. Send 7 bits - WRITE command 
	ack = KT0803L_i2c_send_byte(KT0803L_CMD_WRITE);// Send 0x7C or '0b1111100'
	if (ack == 0) {
		error_nb = 1;
		//return;
	}
	// 2. Send 8 bits register address 
	ack = KT0803L_i2c_send_byte(reg_address);
	if (ack == 0) {
		error_nb = 2;
		//return;
	}
	// 3. Send 8 bits data
	ack = KT0803L_i2c_send_byte(value);
	if (ack == 0) {
		error_nb = 3;
		//return;
	}
	// Send STOP condition 
	KT0803L_i2c_stop_cond();
	
}

//------------------------------------------------
uint8_t KT0803L_ReadRegister(uint8_t reg_address)
{
	uint8_t data_out;
	uint8_t ack = 0;// 0 - error
	
	// Send START condition 
	KT0803L_i2c_start_cond();
	
	// 1. Send 7 bits - WRITE command 
	ack = KT0803L_i2c_send_byte(KT0803L_CMD_WRITE);// Send 0x7C or '0b1111100' 
	if (ack == 0) {
		error_nb = 4;
		//return 0;
	}
	// 2. Send 8 bits register address 
	ack = KT0803L_i2c_send_byte(reg_address);
	if (ack == 0) {
		error_nb = 5;
		//return 0;
	}	
	
	// Restart condition 
	KT0803L_i2c_restart_cond();// OK !!!
	
	// 3. Send 7 bits - READ command 
	ack = KT0803L_i2c_send_byte(KT0803L_CMD_READ);// Send 0x7D or '0b1111101' 
	if (ack == 0) {
		error_nb = 6;
		//return 0;
	}
	// 4. Read 8 bits data 
	data_out = KT0803L_i2c_get_byte(0x01);// NO ACK 
	
	// Send STOP condition 
	KT0803L_i2c_stop_cond();
	
	return data_out; 
}

// KT0803L WRITE Frequency functoin.
void KT0803L_SetFrequency(float freq, uint8_t PA_enable)
{
	if (freq < 70.0) freq = 70.0f;
	if (freq > 108.0) freq = 108.0f;
	// Steps 50 KHz
	volatile uint16_t channel = round(freq * 20);// channel number 1400 - 2160.
	volatile uint16_t ch = channel;
	
	//printf("\nKT0803L Set channel: %d\n", (int)channel);// debug - send to uart 
	
	// Write register 0x02
	uint8_t CHSEL_r02 = (channel & 0x01) << 7;// Get CHSEL[0]
	// Read register 0x02
	uint8_t reg02_value = KT0803L_ReadRegister(0x02);
	reg02_value &= 0x7F;// leave the other bits
	reg02_value |= CHSEL_r02;// add bit CHSEL[0]
	KT0803L_WriteRegister(0x02, reg02_value);
	
	// Write register 0x00
	ch >>= 1;// shift right by 1
	uint8_t reg00_value = (ch & 0xFF);// Get CHSEL[8:1] - CHSEL_r00
	KT0803L_WriteRegister(0x00, reg00_value);
	
	// Write register 0x01
	ch >>= 8;// shift right by 8
	uint8_t CHSEL_r01 = (ch & 0x07);// mask 0b111 - Get CHSEL[11:9] 
	// Read register 0x01
	uint8_t reg01_value = KT0803L_ReadRegister(0x01);
	reg01_value &= 0xF8;// leave the other bits
	reg01_value |= CHSEL_r01;// add bits CHSEL[11:9] 
	KT0803L_WriteRegister(0x01, reg01_value);
	// end write channel 
	
	// PA Bias Current Enhancement - Power PA. 
	// Default 1 Enable PA bias - 0b10 
	uint8_t reg0E_value = 0;
	if (PA_enable == 1) {
		reg0E_value = 2;// Enable PA_BIAS - bit 1
	}
	if (PA_enable == 0) {
		reg0E_value = 0;// Disable PA_BIAS
	}
	KT0803L_WriteRegister(0x0E, reg0E_value);// Write Register 0x0E 
	
	// check error
	if (error_nb > 0) {
		printf("\nerror_nb: %d\n", (int)error_nb);// debug - send to uart 
	}
	error_nb = 0;// clr 
	//
} 

// PA_BIAS 
void KT0803L_setPA_BIAS(uint8_t PA_enable)
{
	// PA_BIAS - PA Bias Current Enhancement. 
	// 0 = Disable PA bias 
	// 1 = Enable PA bias - Default 
	// Register 0x0E (Address: 0x0E, Default: 0x02 
	uint8_t reg0E_value = 0; 
	if (PA_enable == 1) {
		reg0E_value = 2;// Enable PA_BIAS - bit 1 
	}
	if (PA_enable == 0) {
		reg0E_value = 0;// Disable PA_BIAS 
	}
	KT0803L_WriteRegister(0x0E, reg0E_value); // Write Register 0x0E 
}

// KT0803L READ Frequency functoin.
float KT0803L_GetFrequency()
{
	float freq;
	volatile uint16_t channel;
	
	// Read register 0x01
	uint8_t reg01_value = KT0803L_ReadRegister(0x01);
	channel = (uint16_t)(reg01_value & 0x07);// mask 0b111 - Get CHSEL[11:9] 
	channel <<= 8;// shift left by 8
	
	// Read register 0x00
	uint8_t reg00_value = KT0803L_ReadRegister(0x00);// Get CHSEL[8:1] 
	channel |= reg00_value;
	channel <<= 1;// shift left by 1
	
	// Read register 0x02
	uint8_t reg02_value = KT0803L_ReadRegister(0x02);
	channel |= (reg02_value >> 7);// Get CHSEL[0] 
	
	freq = (float)channel * 0.05;// divide by 20
	
	return freq;
} 

float KT0803L_ChannelToFrequency(uint16_t channel)
{
	float freq = (float)channel * 0.05;// divide by 20
	return freq;
} 

uint16_t KT0803L_FrequencyToChannel(float freq)
{
	uint16_t channel = round(freq * 20);
	return channel;
} 

// MUTE
void KT0803L_setMute(uint8_t mute) 
{
	// Software Mute 
	// 0: MUTE Disabled 
	// 1: MUTE Enabled 
	uint8_t data = KT0803L_ReadRegister(0x02);
	// if bit 3 already
	if ((mute == 0x01)  && (data & 0x08) == 0x08) return;// 1
	if ((mute == 0x00) && (data & 0x08) == 0x00) return;// 0
	// flip bit 3
	data = (data ^ 0x08);
	KT0803L_WriteRegister(0x02, data); 
}


uint8_t KT0803L_getMute() 
{
	uint8_t data = KT0803L_ReadRegister(0x02); 
	if ((data & 0x08) == 0x08) { return 1; } else { return 0; } 
}

// RFGAIN - Transmission power setting. 
void KT0803L_setRFGain(uint8_t rfgain)
{
	// RFGAIN maximum value - 0b1111 
	if (rfgain > 15) rfgain = 15;// set bits 0b1111 or 0x0f
	// bits 0 and 1 
	uint8_t data = KT0803L_ReadRegister(0x01);// Read register 0x01 
	data &= 0x3F;// mask 0b111111 
	uint8_t RFGAIN_r01 = (rfgain & 0x03);// mask 0b11 
	data |= (RFGAIN_r01 << 6);// add bits 6 and 7 
	KT0803L_WriteRegister(0x01, data);// Write register 0x01 
	// bit 2 
	data = KT0803L_ReadRegister(0x13);// Read register 0x13 
	data &= 0x7F;// mask 0b1111111 
	uint8_t RFGAIN_r13 = (rfgain & 0x04);// mask 0b100 
	data |=  (RFGAIN_r13 << 5);// shift left by 5, add bit 7. 
	KT0803L_WriteRegister(0x13, data);// Write register 0x13 
	// bit 3 
	data = KT0803L_ReadRegister(0x02);// Read register 0x02 
	data &= 0xBF;// mask 0b10111111 
	uint8_t RFGAIN_r02 = (rfgain & 0x08);// mask 0b1000 
	data |= (RFGAIN_r02 << 3);// shift left by 3, add bit 6. 
	KT0803L_WriteRegister(0x02, data);// Write register 0x02 
	//
}

// Get RFGAIN 
uint8_t KT0803L_getRFGain()
{
	// bit 0, 1 
	uint8_t data = 0;// set 0b00000000 
	uint8_t RFGAIN_r01 = KT0803L_ReadRegister(0x01);// Read register 0x01 
	data |= (RFGAIN_r01 >> 6);// Get RFGAIN[1:0], add bits 0, 1 
	// bit 2 
	uint8_t RFGAIN_r13 = KT0803L_ReadRegister(0x13);// Read register 0x13 
	RFGAIN_r13 &= 0x80;// mask 0b10000000 
	data |= (RFGAIN_r13 >> 5);// Get RFGAIN[2], add bit 2 
	// bit 3 
	uint8_t RFGAIN_r02 = KT0803L_ReadRegister(0x02);// Read register 0x02 
	RFGAIN_r02 &= 0x40;// mask 0b1000000 
	data |= (RFGAIN_r02 >> 3);// Get RFGAIN[3], add bit 3 
	return data; 
}

// PGA Gain for Audio Input 
void KT0803L_setPGA(uint8_t pga)
{
	// Register 0x01 bits 5, 4, 3. PGA[2:0] 
	if (pga > 7) pga = 7;// 0b111 
	uint8_t data = KT0803L_ReadRegister(0x01); 
	data &= 0xC7;// mask 0b11000111 
	data |= (pga << 3); 
	KT0803L_WriteRegister(0x01, data); 
}

uint8_t KT0803L_getPGA()
{
	uint8_t data = KT0803L_ReadRegister(0x01); 
	data = (data >> 3) & 0x07; 
	return data; 
}

// Pre-emphasis Time Constant 
void KT0803L_setPHTCNST(uint8_t value)
{
	// Pre-emphasis Time Constant: 1 - 50 us (Europe, Australia), 0 - 75 us (USA, Japan). 
	// Default PHTCNST 0 - 75 us (USA, Japan). 
	uint8_t data = KT0803L_ReadRegister(0x02); 
	// if bit 0 already 
	if ((value == 0x01)  && (data & 0x01) == 0x01) return; 
	if ((value == 0x00) && (data & 0x01) == 0x00) return; 
	// flip bit 0 
	data = data ^ 0x01; 
	KT0803L_WriteRegister(0x02, data); 
}

uint8_t KT0803L_getPHTCNST()
{
	uint8_t data = KT0803L_ReadRegister(0x02); 
	data &= 0x01; 
	return data; 
}

// Pilot Tone Amplitude Adjustment 
void KT0803L_setPilotToneAdjust(uint8_t mode)
{
	// PLTADJ bit 
	// Pilot Tone Amplitude Adjustment 
	// 0: Amplitude low 
	// 1: Amplitude high 
	// Default 0 
	if (mode > 1) mode = 1; 
	uint8_t data = KT0803L_ReadRegister(0x02); 
	// is bit 2 already 
	if ((mode == 0x01) && (data & 0x04) == 0x04) return; 
	if ((mode == 0x00) && (data & 0x04) == 0x00) return; 
	// flip bit 2 
	data = data ^ 0x04;// ^ 0b100 
	KT0803L_WriteRegister(0x02, data); 
}

uint8_t KT0803L_getPilotToneAdjust()
{
	uint8_t data = KT0803L_ReadRegister(0x02); 
	data &= 0x04;// mask 0b100 
	if (data == 0x04) { return 1; } else { return 0; } 
}

//-----------------------------------------------------------
// Specific functions 
void KT0803L_setMono()
{
	// MONO bit 6 - Default 0 = Stereo OR 1 = Mono
	uint8_t data = KT0803L_ReadRegister(0x04); 
	if ((data & 0x40) == 0x00) // if Stereo
	{
		// Set bit 6 
		data |= 0x40;// 0b1000000 
		KT0803L_WriteRegister(0x04, data); 
	} 
}

void KT0803L_setStereo()
{
	// MONO bit 6 - Default 0 = Stereo OR 1 = Mono
	uint8_t data = KT0803L_ReadRegister(0x04); 
	if ((data & 0x40) == 0x40) // if set bit 0b1000000 - Mono
	{
		data &= ~(0x40);// Clear bit 6 OR flip bit 6 // data = data ^ (0x40);
		KT0803L_WriteRegister(0x04, data); 
	} 
}

uint8_t KT0803L_isStereo()
{
	// MONO bit 6 - Default 0 = Stereo OR 1 = Mono
	uint8_t data = KT0803L_ReadRegister(0x04);
	if ((data & 0x40) == 0x00) {
		return 1;// Stereo 
	} else {
		return 0;// Mono 
	} 
}

// BASS 
void KT0803L_setBass(uint8_t bass)
{
	// BASS[1:0] - Bass Boost Control 
	// 00 : Disabled - Default 
	// 01 : 5dB 
	// 10 : 11dB 
	// 11 : 17dB 
	if (bass > 3) bass = 3;// set 0b11 
	uint8_t data = KT0803L_ReadRegister(0x04);// Read Register 0x04 
	data = (data & 0xFC);// mask 0b11111100 
	data |= bass;// add bits 0, 1. 
	KT0803L_WriteRegister(0x04, data);// Write Register 0x04 
}

uint8_t KT0803L_getBass()
{
	uint8_t data = KT0803L_ReadRegister(0x04);// Read Register 0x04 
	uint8_t bass = data & 0x03;// mask 0b11 
	return bass; 
}

// PW_OK 
uint8_t KT0803L_powerOK()
{
	// PW_OK - bit 4 
	uint8_t data = KT0803L_ReadRegister(0x0F);// Register 0x0F - Read only 
	if ((data & 0x10) == 0x10) { // if set bit 0b10000 
		return 1; 
	} else { 
		return 0; 
	} 
}

// SLNCID 1 when Silence is Detected
uint8_t KT0803L_isSilenceDetected()
{
	// SLNCID - bit 2 
	uint8_t data = KT0803L_ReadRegister(0x0F);// Register 0x0F - Read only 
	if ((data & 0x04) == 0x04) { // if set bit 0b100 
		return 1; 
	} else { 
		return 0; 
	} 
}

// SLNCDIS Silence Detection Disable. 
void KT0803L_SilenceDetectionDisable()
{
	// SLNCDIS bit 7 
	// 0 : Enable 
	// 1 : Disable - Default 
	uint8_t data = KT0803L_ReadRegister(0x12);// Read Register 0x12 
	uint8_t SLNCDIS_r12 = (data & 0x80);// 0b10000000 or (1 << 7) 
	if (SLNCDIS_r12 == 0x00) // if Enable detection - if bit 7 cleared  
	{
		data |= (1 << 7);// Set bit 7 - Disable Silence Detection 
		KT0803L_WriteRegister(0x12, data); 
	} 
}


// Main Tunings 
void KT0803L_Set_Main_Tunings()
{
	//float FM_frequency = 97.10f;// Frequency MHz. Channel 1942. 
	// PA Bias Current Enhancement.
	//uint8_t PA_bias = 0;// 0 = Disable PA bias. 1 = Enable PA bias 1 - Default. 
	// Pre-emphasis Time Constant: 1 - 50 us (Europe, Australia), 0 - 75 us (USA, Japan). 
	//uint8_t Ptime = 1;// Set 1 - 50 us (Europe, Australia). Default 0 - 75 us (USA, Japan).
	//uint8_t mute = 0x00;// 0x00 - Disabled Default, 0x01 - Enabled.
	
	KT0803L_i2c_init();
	//KT0803L_setPHTCNST(Ptime);
	KT0803L_setMono();
	//KT0803L_setMute(mute);
	//KT0803L_SetFrequency(FM_frequency, PA_bias); 
	
	// RFGAIN 
	//uint8_t RF_Gain_def = KT0803L_getRFGain();
	//printf("Get RF_Gain: %lu\n", (uint32_t)RF_Gain_def);// default 15. 
	
	// Set RFGAIN 
	uint8_t RF_Gain = 0;// 0b0000 RFOUT 95.5 dBuV 
	//RF_Gain = 7;//       0b0111 RFOUT 102.8 dBuV 
	//RF_Gain = 15;//      0b1111 RFOUT 108 dBuV (112.5dBuV, PA_BIAS=1) - default setting 
	
	KT0803L_setRFGain(RF_Gain);
	
	// Read RFGAIN 
	RF_Gain = KT0803L_getRFGain();
	printf("Get RF_Gain: %lu (RFOUT 95.5 dBuV)\n", (uint32_t)RF_Gain);
}

/*
void KT0803L_Test01()
{
	// Test Read RFGAIN 
	uint16_t tmpreg;
	KT0803L_i2c_init();
	tmpreg = KT0803L_getRFGain();
	printf("RFGAIN: %lu \n", (uint32_t)tmpreg);
	tmpreg = KT0803L_getPHTCNST();
	printf("PHTCNST: %lu \n", (uint32_t)tmpreg);
	tmpreg = KT0803L_getMute();
	printf("MUTE: %lu \n", (uint32_t)tmpreg);
	// Test Read Frequency 
	float freq_val;
	freq_val = KT0803L_GetFrequency();
	tmpreg = KT0803L_FrequencyToChannel(freq_val);
	printf("Channel: %lu \n", (uint32_t)tmpreg);
	//
	tmpreg = 1890;// 94.50 MHz. 
	freq_val = KT0803L_ChannelToFrequency(tmpreg);
	//printf("FM Frequency: %.2f \n", freq_val);
	tmpreg = KT0803L_FrequencyToChannel(freq_val);// to channel 
	printf("Channel 1890: %lu \n", (uint32_t)tmpreg);
}
*/

//-----------------------------------------------------------
//-----------------------------------------------------------

/* USER CODE END 0 */

