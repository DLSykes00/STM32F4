#include "stm32f4xx.h"

static I2C_HandleTypeDef I2C_Params; // Structure handle for I2C parameters
static I2S_HandleTypeDef I2S_Params; // Structure handle for I2S parameters

static void Audio_I2C_Init()
{
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // Enable I2C1 Clock

	I2C_Params.Instance = I2C1; // Select I2C1
	I2C_Params.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT; // 7-bit addressing mode
	I2C_Params.Init.ClockSpeed = 100000; // Set I2C clock speed (Hz)
	I2C_Params.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE; // Disable dual address
	I2C_Params.Init.DutyCycle = I2C_DUTYCYCLE_2; // Set duty cycle
	I2C_Params.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE; // Disable general call
	I2C_Params.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE; // Disable nostretch
	I2C_Params.Init.OwnAddress1 = 0; // Set own address

	HAL_I2C_Init(&I2C_Params); // Initialise I2C1
	__HAL_I2C_ENABLE(&I2C_Params); // Enable I2C1
}

static void Audio_I2S_Init()
{
	RCC->APB1ENR |= RCC_APB1ENR_SPI3EN; // Enable clock used for I2S3

	I2S_Params.Instance = SPI3; // Select SPI3 (I2S3)
	I2S_Params.Init.Mode = I2S_MODE_MASTER_TX; // Select stm as master
	I2S_Params.Init.Standard = I2S_STANDARD_PHILIPS; // Standard phillips spec
	I2S_Params.Init.DataFormat = I2S_DATAFORMAT_16B; // 16-bit data 
	I2S_Params.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE; // Enable master clock
	I2S_Params.Init.AudioFreq = I2S_AUDIOFREQ_48K; // 48 KHz audio freq
	I2S_Params.Init.CPOL = I2S_CPOL_LOW; // Clock polarity low
	I2S_Params.Init.ClockSource = I2S_CLOCK_PLL; // Use PLL clock source
	I2S_Params.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE; // Disable full duplex
	HAL_I2S_Init(&I2S_Params); // Initialise I2S3

	__HAL_I2S_ENABLE(&I2S_Params);  // Enable I2S3
}

static void Audio_GPIO_Init()
{
	GPIO_InitTypeDef GPIOA_Params;	// Structure handle for GPIOA parameters (I2S1)
	GPIO_InitTypeDef GPIOB_Params;  // Structure handle for GPIOB parameters (I2C1)
	GPIO_InitTypeDef GPIOC_Params;  // Structure handle for GPIOC parameters (I2S3)
	GPIO_InitTypeDef GPIOD_Params;  // Structure handle for GPIOD parameters (CS43L22 Reset)

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable the clock for GPIOA
	GPIOA_Params.Pin = GPIO_PIN_4; // Pin 4 (LRCK)
	GPIOA_Params.Alternate = GPIO_AF6_SPI3; // Alternate function for SPI3/I2S3
	GPIOA_Params.Mode = GPIO_MODE_AF_PP; // Alternate function push-pull mode
	GPIOA_Params.Speed = GPIO_SPEED_FAST; // Fast freqency
	GPIOA_Params.Pull = GPIO_NOPULL; // No pull-up or pull-down activation
	HAL_GPIO_Init(GPIOA, &GPIOA_Params); // Intialise GPIOA

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Enable the clock for GPIOB
	GPIOB_Params.Pin = GPIO_PIN_6 | GPIO_PIN_9; // Pins 6 (SCL), 9 (SDA)
	GPIOB_Params.Alternate = GPIO_AF4_I2C1; // Alternate function for I2C1
	GPIOB_Params.Mode = GPIO_MODE_AF_OD; // Alternate function open drain mode
	GPIOB_Params.Speed = GPIO_SPEED_FAST; // Fast freqency
	GPIOB_Params.Pull = GPIO_NOPULL; // No pull-up or pull-down activation
	HAL_GPIO_Init(GPIOB, &GPIOB_Params); // Intialise GPIOB

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // Enable the clock for GPIOC
	GPIOC_Params.Pin = GPIO_PIN_7 | GPIO_PIN_10 | GPIO_PIN_12; // Select pins 7 (MCLK), 10 (SCLK), 12 (SDIN)
	GPIOC_Params.Alternate = GPIO_AF6_SPI3; // Select alternate function for SPI3/I2S3 
	GPIOC_Params.Mode = GPIO_MODE_AF_PP; // Alternate function push-pull mode
	GPIOC_Params.Speed = GPIO_SPEED_FAST; // Fast freqency
	GPIOC_Params.Pull = GPIO_NOPULL; // No pull-up or pull-down activation
	HAL_GPIO_Init(GPIOC, &GPIOC_Params); // Intialise GPIOC

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; //Enable the clock for GPIOD
	GPIOD_Params.Pin = GPIO_PIN_4; // Pin 4 (CS43L22 Reset)
	GPIOD_Params.Mode = GPIO_MODE_OUTPUT_PP; // Selects normal push-pull mode
	GPIOD_Params.Speed = GPIO_SPEED_FAST; // Selects fast speed 
	GPIOD_Params.Pull = GPIO_NOPULL; // Selects pull-down activation
	HAL_GPIO_Init(GPIOD, &GPIOD_Params); // Intialise GPIOD
}

static void Audio_Registers_Init()
{
	uint8_t data[2]; // data[0] = register address, data[1] = register value

	// Config DAC
	GPIOD->BSRR |= GPIO_PIN_4; // Set reset high
	
	data[0] = 0x02; data[1] = 0x01; // Set power down mode
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
	
	data[0] = 0x04; data[1] = 0xAF; // Set headphones always on, speaker always off
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
	
	data[0] = 0x1C; data[1] = 0x73; // 1000Hz, 1.2s on time (beep generator)
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
	
	data[0] = 0x1D; data[1] = 0x00; // 1.2s off, -6 dB gain (beep generator)
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
	
	data[0] = 0x1E; data[1] = 0xE0; // Continuous beep (beep generator)
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
}

void Audio_Init(void)
{
	Audio_I2C_Init();
	Audio_I2S_Init();
	Audio_GPIO_Init();
	Audio_Registers_Init();
}

void Audio_Power_On()
{
	uint8_t data[2]; // data[0] = register address, data[1] = register value
	uint8_t rData[1]; // Recieved register data (from a read)
	
	// Start DAC (recommened power-up sequence) (Section 4.9/4.11 CS43L22 datasheet)
	data[0] = 0x00; data[1] = 0x99;
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
	
	data[0] = 0x47; data[1] = 0x80; // 
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
	
	data[0] = 0x32;
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 1, 1000);
	HAL_I2C_Master_Receive(&I2C_Params, 0x94, rData, 1, 1000);
	rData[0] |= 0x80; data[1] = rData[0];
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
	data[1] &= ~0x80;
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
	
	data[0] = 0x00; data[1] = 0x00;
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
	
	data[0] = 0x02; data[1] = 0x9E; // Set power up mode
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
}

void Audio_Power_Off()
{
	// TODO: Fully follow recommeneded power-down sequence from CS43L22 datasheet 4.10 
	
	uint8_t data[2]; // data[0] = register address, data[1] = register value
	
	data[0] = 0x02; data[1] = 0x01; // Set power down mode
	HAL_I2C_Master_Transmit(&I2C_Params, 0x94, data, 2, 1000);
}
