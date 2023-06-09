#ifndef LED_H
#define LED_H

#include "stm32f4xx.h"

// Associated pins for LEDs
#define LED_GREEN 12
#define LED_ORANGE 13
#define LED_RED 14
#define LED_BLUE 15

#define LED_ON 1
#define LED_OFF 0

void LED_Init(void);
void LED_Set(uint8_t LED_Pin, uint8_t LED_State);
uint8_t LED_isSet(uint8_t LED_Pin);

#endif
