#include "stm32f4xx.h"

void Button_Init()
{
	GPIO_InitTypeDef GPIOA_Params;
	
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable port A clock for user buton.
	GPIOA_Params.Pin = GPIO_PIN_0; // Select pin 0
	GPIOA_Params.Mode = GPIO_MODE_IT_RISING; // Select interrupt trigger mode
	GPIOA_Params.Pull = GPIO_NOPULL; // No pull-up/pull-down
	GPIOA_Params.Speed = GPIO_SPEED_FAST; // Fast frequency 
	HAL_GPIO_Init(GPIOA, &GPIOA_Params); // Initialise GPIOA
	
	// Hardware interupt
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Enable interrupt clock
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0x00, 0x00); // Set highest priority
	HAL_NVIC_EnableIRQ(EXTI0_IRQn); // Enable ISR
}

char Button_State()
{
	return((GPIOA->IDR & 0x00000001) == 0x00000001); // Check input data register for button
}
