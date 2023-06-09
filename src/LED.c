#include "stm32f4xx.h"

void LED_Init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; // Enable port D clock for user LEDs
	GPIOD->MODER |= GPIO_MODER_MODER12_0 | GPIO_MODER_MODER13_0 |
	GPIO_MODER_MODER14_0 | GPIO_MODER_MODER15_0; // Set user LEDs to output mode
}

void LED_Set(uint8_t LED_Pin, uint8_t LED_State) 
{
	// Put 1 in correct bit set-reset register position to enable/disable selected LED
	if (LED_State == 1) GPIOD->BSRR = 1 << LED_Pin;
	else GPIOD->BSRR = 1 << (LED_Pin + 16);
}

char LED_isSet(uint8_t LED_Pin) 
{
	// Check output data register for selected LED
	return ((GPIOD->ODR & (1 << LED_Pin)) == (1 << LED_Pin)); 
}
