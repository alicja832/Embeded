#include "cmsis_os2.h"
#include "Board_LED.h"                  // ::Board Support:LE
#include "RTE_Device.h"                 // Keil::Device:Startup
#include "rtx_os.h"

void Timer1_Callback (void *arg)
{
	LED_On(3);
	
}	// prototypes for timer callback function

void Timer2_Callback (void *arg)
{
	LED_Off(3);
	
}	// prototypes for timer callback function

void app_main(void * tmp)
{
	uint32_t exec1= 1U; 
	
    osTimerId_t id1 = osTimerNew(Timer1_Callback, osTimerPeriodic, &exec1,NULL);
	osTimerId_t id2 = osTimerNew(Timer2_Callback, osTimerPeriodic, &exec1,NULL);

	uint32_t timerDelay = 2000U;
	
	osTimerStart(id1,timerDelay);
	osDelay(1000);
	osTimerStart(id2,timerDelay);
	while(1)
	{
		
		osDelay(1000);
	}
	
}
int main()
{
	
	SystemCoreClockUpdate();
	osKernelInitialize();
	
	//wtedy sie wszystkie diody sie swieca
	LED_Initialize();
	void *thread = osThreadNew(app_main,NULL,NULL);
	osKernelStart();
	while(1)
	{
		
	}
}
