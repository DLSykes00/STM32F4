#include "cmsis_os.h"                                           // CMSIS RTOS header file

#include "Accelerometer.h"
#include "Audio.h"
#include "Button.h"
#include "LED.h"
#include "Timer.h"

static char state = 1; // 1 = Running, 0 = Paused

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
void Active_Thread (void const *argument);                             // thread function
osThreadId tid_Active_Thread;                                          // thread id
osThreadDef (Active_Thread, osPriorityNormal, 1, 0);                   // thread object

void Paused_Thread (void const *argument);                             // thread function
osThreadId tid_Paused_Thread;                                          // thread id
osThreadDef (Paused_Thread, osPriorityNormal, 1, 0);                   // thread object

void Button_Thread (void const *argument);                             // thread function
osThreadId tid_Button_Thread;                                          // thread id
osThreadDef (Button_Thread, osPriorityNormal, 1, 0);                   // thread object


int Init_Active_Thread (void) 
{
	tid_Active_Thread = osThreadCreate (osThread(Active_Thread), NULL);
	if (!tid_Active_Thread) return(-1);

	return(0);
}

void Active_Thread (void const *argument) 
{
	// Ensure all LEDs are off
	LED_Set(LED_GREEN, LED_OFF);
	LED_Set(LED_ORANGE, LED_OFF);
	LED_Set(LED_RED, LED_OFF);
	LED_Set(LED_BLUE, LED_OFF);
	
	Audio_Power_Off();
	
	Accelerometer_Power_On();
	Timer_Enable();
	
	while (1) 
	{
		// Wait for new accelerometer data to be available
		osSignalWait(0x01, osWaitForever);
		
		// Set LEDs depending on accelerometer output data
		Accelerometer_UpdateLED();
		
		osThreadYield ();                                           // suspend thread
	}
}


int Init_Paused_Thread (void) 
{
	tid_Paused_Thread = osThreadCreate (osThread(Paused_Thread), NULL);
	if (!tid_Paused_Thread) return(-1);

	return(0);
}

void Paused_Thread (void const *argument) 
{
	// Ensure all LEDs are off
	LED_Set(LED_GREEN, LED_OFF);
	LED_Set(LED_ORANGE, LED_OFF);
	LED_Set(LED_RED, LED_OFF);
	LED_Set(LED_BLUE, LED_OFF);
	
	Accelerometer_Power_Off();
	
	Audio_Power_On();
	
	// Cycle through paused LED sequence
	while (1) 
	{
		LED_Set(LED_ORANGE, LED_ON);
		osDelay(250);
		LED_Set(LED_ORANGE, LED_OFF);
		
		LED_Set(LED_RED, LED_ON);
		osDelay(250); 
		LED_Set(LED_RED, LED_OFF);
		
		LED_Set(LED_BLUE, LED_ON);
		osDelay(250);
		LED_Set(LED_BLUE, LED_OFF);
		
		LED_Set(LED_GREEN, LED_ON);
		osDelay(250);
		LED_Set(LED_GREEN, LED_OFF);
		
		osThreadYield ();                                           // suspend thread
	}
}


int Init_Button_Thread (void) 
{
	tid_Button_Thread = osThreadCreate (osThread(Button_Thread), NULL);
	if (!tid_Button_Thread) return(-1);

	return(0);
}

void Button_Thread (void const *argument) 
{
	while (1) 
	{	
		// Wait for button press (from interrupt)
		osSignalWait(0x01, osWaitForever);
		
		// Debounce button 
		osDelay(20); // Wait 20ms for debounce
		while(Button_State() == Button_ON); // Wait until button released
		
		// Toggle the state (active/paused)
		if (state) // If active
		{
			Timer_Disable(); // Timer disabled before terminating thread as timer ISR sets active thread flag
			osThreadTerminate(tid_Active_Thread);
			Init_Paused_Thread();
			state = 0;
		}
		else // If paused
		{
			osThreadTerminate(tid_Paused_Thread);
			Init_Active_Thread();
			state = 1;
		}
		
		osDelay(100); // Wait 100ms before allowing another state change
		
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0); // Clear any interrupts that may have occured
		HAL_NVIC_EnableIRQ(EXTI0_IRQn); // Re-enable interrupt for user button

		osThreadYield ();                                           // suspend thread
	}
}

// User button ISR
void EXTI0_IRQHandler(void)
{
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) == SET) // Checks to see if the interrupt line has been set
	{ 
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0); // Interrupt cleared
		HAL_NVIC_DisableIRQ(EXTI0_IRQn); // Prevent any input bounces re-triggering ISR
		
		osSignalSet(tid_Button_Thread, 0x01); // Let button thread know user has pushed button
	}
}

// TIM2 ISR used to poll accelerometer at TIM2 Hz
void TIM2_IRQHandler(void)
{
	TIM2->SR &= ~TIM_SR_UIF; // Clear timer interrupt flag
	
	osSignalSet(tid_Active_Thread, 0x01);	// Let active thread know to update
}
