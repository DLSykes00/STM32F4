// Date: Jan 2022

/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "Thread.h"

#include "Accelerometer.h"
#include "Audio.h"
#include "Button.h"
#include "LED.h"
#include "Timer.h"

/*
 * main: initialize and start the system
 */
int main (void) 
{
	SystemCoreClockUpdate();
	osKernelInitialize ();                    // initialize CMSIS-RTOS

	// initialize peripherals here
	Accelerometer_Init();
	LED_Init();
	Button_Init();
	Timer_Init();
	Audio_Init();

	// create 'thread' functions that start executing,
	Init_Active_Thread();
	Init_Button_Thread();

	osKernelStart ();                         // start thread execution 

	while(1);
}
