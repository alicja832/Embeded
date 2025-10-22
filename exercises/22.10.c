#include "RTE_Device.h"                 // Keil::Device:Startup
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "LPC17xx.h"                    // Device header
#include "cmsis_os2.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "rtx_os.h"
#include "string.h"
#include <stdio.h>

#define FLAGS_MSK1 0x00000001U

//jest zamontowany USART1 na plytce
extern ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART0;
char cmd;
uint32_t counter;

uint32_t active;

osEventFlagsId_t evt_id;

//Timer 0/1/2/3 Basic Configuration,Dokumentacja Startupu->Reference->Interrupts and Exceptions
void TIMER0_IRQHandler(void)  {
    /* Timer 0 interrupt handler  */
	LPC_TIM0->IR |= (1 << 0);
	osEventFlagsSet(evt_id, FLAGS_MSK1);
}

void app_main(void * tmp)
{
	
	if (!tmp)
		while(1);
	//druga lampka miga-zapala sie i wlacza przy kazdej petli
	uint32_t led_idx = ((uint32_t*)tmp)[0];
	uint32_t frequency = ((uint32_t*)tmp)[1];

	uint32_t flags = 0x00000000U;
	char test2[20];
	
	while(1)
	{
		flags = osEventFlagsWait(evt_id, FLAGS_MSK1, osFlagsWaitAny, osWaitForever);
		sprintf(test2, "Ping %08x\r\n", flags);
		USARTdrv->Send(test2, strlen(test2));
		osDelay(frequency);
		
	}
}
void create_threads(void * tmp)
{
	
	uint16_t x = 1;
	void *thread = &x;
	char test2[20];
	
	while(thread)
	{
		//++counter;
		//test2[0]=(char)counter-'0';
		//sprintf(test2, "test %d\n", counter);
		//USARTdrv->Send("\n OK \n",7);
		//USARTdrv->Send(test2,strlen(test2));
		osDelay(100);
		//thread = osThreadNew(app_main,NULL,NULL);
	}
	while(1);
}
int main(void)
{
	SystemCoreClockUpdate();
	osKernelInitialize();
	//LED_Initialize();
	uint32_t data_1[] = {2,100};
	uint32_t data_2[] = {3,1000};
	uint32_t data_3[] = {1,10000};
	uint32_t data_4[] = {0,200};
	void *tmpdata=NULL;
	//uint32_t data_5[3000000];
	//przesylanie wiadomosci na terminal 
	USARTdrv->Initialize(NULL);
    /*Power up the USART peripheral */
	USARTdrv->PowerControl(ARM_POWER_FULL);
    /*Configure the USART to 4800 Bits/sec */
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 4800);
     
    /* Enable Receiver and Transmitter lines */
   USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
   USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
 
	
	LPC_TIM0->PR=250000;
    LPC_TIM0->MR0=50;
    LPC_TIM0->MCR=3;
	
	NVIC_EnableIRQ(TIMER0_IRQn);
		//wlaczanie timera
    LPC_TIM0->TCR = 1;
    active = NVIC_GetActive(TIMER0_IRQn);
	
	evt_id = osEventFlagsNew(NULL);
	
	void *thread = osThreadNew(app_main,data_1,NULL);
	
	//osThreadNew(create_threads,NULL,NULL);
	
	
	osKernelStart();
	while(1)
	{
		
	}
}
