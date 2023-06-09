#include "stm32f4xx.h"

void Timer_Init(void)
{
	const uint8_t Hz = 25; // Match's output data rate of LIS3DSH
	
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable clock for timer 2
	TIM2->PSC = 8400-1; // Prescaler value for 10KHz counter freq
	TIM2->ARR = (10000-1) / Hz; // Auto-reload register value (1 / Hz) seconds
	TIM2->DIER = TIM_DIER_UIE; // Enable interrupts
	TIM2->EGR = 1; // Reinitialise timer
	
	// Hardware interupt
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Enable interrupt clock
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
	HAL_NVIC_SetPriority(TIM2_IRQn, 0x00, 0x00); // Set highest priority
	HAL_NVIC_EnableIRQ(TIM2_IRQn); // Enable ISR
}

void Timer_Enable()
{
	TIM2->CR1 |= 1; // Control register 1 enable timer
}

void Timer_Disable()
{
	TIM2->CR1 &= ~1; // Control register 1 disable timer
}
