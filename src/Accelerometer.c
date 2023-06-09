#include "stm32f4xx.h"

#include "LED.h"

// Declarations
static SPI_HandleTypeDef SPI1_Params; // Structure handle for SPI1 parameters
static uint32_t SPI_data_timeout = 1000; // SPI maximum wait time

static void Accelerometer_SPI_Init(void) 
{
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // Enable the clock for SPI1
	
	SPI1_Params.Instance = SPI1; // Select SP1
	SPI1_Params.Init.Mode = SPI_MODE_MASTER; // Set master mode 
	SPI1_Params.Init.NSS = SPI_NSS_SOFT; // Software slave mode
	SPI1_Params.Init.Direction = SPI_DIRECTION_2LINES; // Full-duplex mode
	SPI1_Params.Init.DataSize = SPI_DATASIZE_8BIT; // 8-bit (1 byte) transmission size
	SPI1_Params.Init.CLKPolarity = SPI_POLARITY_HIGH; // Clock idle when line high
	SPI1_Params.Init.CLKPhase = SPI_PHASE_2EDGE; // Data read on 2nd transition of clock
	SPI1_Params.Init.FirstBit = SPI_FIRSTBIT_MSB; // Most significant bit first
	SPI1_Params.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32; // Divides main clock freqency by 32 for SPI1
	HAL_SPI_Init(&SPI1_Params); // Initialise SPI1 with parameters defined above
	
	__HAL_SPI_ENABLE(&SPI1_Params); // Enable SPI1
}

static void Accelerometer_GPIO_Init(void) 
{
	GPIO_InitTypeDef GPIOA_Params; 
	GPIO_InitTypeDef GPIOE_Params;

	// SPI GPIO Setup
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable GPIO port A clock
	GPIOA_Params.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7; // Select pins 5 (SCL), 6 (SDO), 7 (SDI)
	GPIOA_Params.Alternate = GPIO_AF5_SPI1; // Set SPI1 alternate function
	GPIOA_Params.Mode = GPIO_MODE_AF_PP; // Alternate function push-pull mode
	GPIOA_Params.Speed = GPIO_SPEED_FAST; // Fast frequency 
	GPIOA_Params.Pull = GPIO_NOPULL; // No pull-up/pull-down
	HAL_GPIO_Init(GPIOA, &GPIOA_Params); // Initialise GPIOA
	
	// LIS3DSH SPI comm chip select pin GPIO setup
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; // Enable GPIO port E clock
	GPIOE_Params.Pin = GPIO_PIN_3; // Select Pin 3 (CS_SPI)
	GPIOE_Params.Mode = GPIO_MODE_OUTPUT_PP; // Output push-pull mode
	GPIOE_Params.Speed = GPIO_SPEED_FAST; // Fast frequency 
	GPIOE_Params.Pull = GPIO_PULLUP; // Pull-up activation
	HAL_GPIO_Init(GPIOE, &GPIOE_Params); // Initialise GPIOE
	
	GPIOE->BSRR = GPIO_PIN_3; // Set pin 3 high (LIS3DSH SPI idle)
}

void Accelerometer_Power_On()
{
	uint8_t data[2]; // data[0] = register address, data[1] = register value
	
	data[0] = 0x20; data[1] = 0x43; // Control Register 4 (20h) set to 25hz and x/y axis enabled.
	GPIOE->BSRR = GPIO_PIN_3 << 16; // Signal start of SPI
	HAL_SPI_Transmit(&SPI1_Params, data, 2, SPI_data_timeout); // Transmit bytes
	GPIOE->BSRR = GPIO_PIN_3; // Signal end of SPI
}

void Accelerometer_Power_Off()
{
	uint8_t data[2]; // data[0] = register address, data[1] = register value
	
	data[0] = 0x20; data[1] = 0x00; // Control Register 4 (20h) set to power down LIS3DSH
	GPIOE->BSRR = GPIO_PIN_3 << 16; // Signal start of SPI
	HAL_SPI_Transmit(&SPI1_Params, data, 2, SPI_data_timeout); // Transmit bytes 	
	GPIOE->BSRR = GPIO_PIN_3; // Signal end of SPI 
}

void Accelerometer_Init()
{
	Accelerometer_GPIO_Init();
	Accelerometer_SPI_Init();
}

void Accelerometer_UpdateLED()
{
	uint8_t axis_data_address[1] = {0x80 | 0x28}; // [Read]/Write Bit | Start address for the 6 output registers.
	uint8_t axis_data_buffer[4]; // 2 bytes data for each axis (x, y)
	
	int16_t x_value; // Store x-axis bits
	int16_t y_value; // Store y-axis bits
	int16_t deadzone = 3000; // Prevent LED flickering around equilibrium point with a deadzone
	
	GPIOE->BSRR = GPIO_PIN_3 << 16; // Signal start of SPI
	HAL_SPI_Transmit(&SPI1_Params, axis_data_address, 1, SPI_data_timeout); // Transmit address	
	HAL_SPI_Receive(&SPI1_Params, axis_data_buffer, 6, SPI_data_timeout); // Read registers
	GPIOE->BSRR = GPIO_PIN_3; // Signal end of SPI
	
	// Combine two 8-bit register values into single signed 16-bit value for each axis.
	x_value = (axis_data_buffer[1] << 8) | (axis_data_buffer[0] & 0xFF);
	y_value = (axis_data_buffer[3] << 8) | (axis_data_buffer[2] & 0xFF);
	
	// Logic to set LEDs depending on oritentation of LIS3DSH x and y axis.
	if (x_value > deadzone)
	{	
		LED_Set(LED_RED, LED_ON);
		LED_Set(LED_GREEN, LED_OFF);
	}
	else if (x_value < -deadzone)
	{
		LED_Set(LED_GREEN, LED_ON);
		LED_Set(LED_RED, LED_OFF);
	}
	else if (x_value < (deadzone / 2) && (x_value > -deadzone / 2))
	{
		LED_Set(LED_RED, LED_OFF);
		LED_Set(LED_GREEN, LED_OFF);
	}
	
	if (y_value > deadzone)
	{ 
		LED_Set(LED_ORANGE, LED_ON);
		LED_Set(LED_BLUE, LED_OFF);
	}
	else if (y_value < -deadzone)
	{
		LED_Set(LED_BLUE, LED_ON);
		LED_Set(LED_ORANGE, LED_OFF);
	}
	else if ((y_value < deadzone / 2) && (y_value > -deadzone / 2))
	{
		LED_Set(LED_BLUE, LED_OFF);
		LED_Set(LED_ORANGE, LED_OFF);
	}
}
