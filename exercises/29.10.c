#include "RTE_Device.h"                 // Keil::Device:Startup
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "LPC17xx.h"                    // Device header
#include "cmsis_os2.h"
//#include "Board_LED.h"                  // ::Board Support:LED
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
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
char test111[4] = {'o', 'e', 'i', '\0'};
const char *x="Ala ma kota";

osEventFlagsId_t evt_id;

uint32_t osRtxErrorNotify (uint32_t code, void *object_id) {

	while(*x!='\0')
		{
			LPC_UART0->THR=*x;
			x++;
		}


	while(1)
		{
			if(LPC_UART0->LSR&1)
				LPC_UART0 -> THR = LPC_UART0 -> RBR;
		}
		
//return 0U;
}

void app_main(void * tmp)
{
	
	//druga lampka miga-zapala sie i wlacza przy kazdej petli
	//uint32_t led_idx = ((uint32_t*)tmp)[0];
	//uint32_t frequency = ((uint32_t*)tmp)[1];
	uint32_t frequency = 1000;

	uint32_t flags = 0x00000000U;
	char test2[20] = {'o', 'e', 'i', '\0'};
	
	//osEventFlagsSet(evt_id, FLAGS_MSK1);
	
	while(1)
	{
		//flags = osEventFlagsWait(evt_id, FLAGS_MSK1, osFlagsWaitAny, osWaitForever);
		//USARTdrv->Send(test2, strlen(test2));
		//sprintf(test2, "Ping %08x\r\n", flags);
		//USARTdrv->Send(test2, strlen(test2));
		osDelay(frequency);
	}
}

void overflow_thread(void * tmp)
{
	
	uint32_t frequency = 1000;
	volatile uint16_t ehe = 0;
	volatile uint16_t temp1[1024]={0};
	
	while(1)
	{
		
		osDelay(frequency);
		memset(temp1, 8, 1024);
		osDelay(1);
	
	}
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
	//USARTdrv->Initialize(NULL);
    /*Power up the USART peripheral */
	//USARTdrv->PowerControl(ARM_POWER_FULL);
    /*Configure the USART to 4800 Bits/sec */
	//USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                     // ARM_USART_DATA_BITS_8 |
                     // ARM_USART_PARITY_NONE |
                     // ARM_USART_STOP_BITS_1 |
                     // ARM_USART_FLOW_CONTROL_NONE, 4800);
     
    /* Enable Receiver and Transmitter lines */
   //USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
   //USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
	
	PIN_Configure(0,2,1,0,0);
    PIN_Configure(0,3,1,0,0);

    LPC_UART0->LCR=3|(1<<7);
    LPC_UART0->DLL=27;
    LPC_UART0->DLM=0;
    LPC_UART0->LCR=3;

    LPC_UART0->LCR=3;

	//osRtxErrorNotify(22, NULL);
	
	evt_id = osEventFlagsNew(NULL);
	
	void *thread = osThreadNew(app_main,NULL,NULL);
	void *thread2 = osThreadNew(overflow_thread,NULL,NULL);
	
	//osThreadNew(create_threads,NULL,NULL);
	
	
	osKernelStart();
	while(1)
	{
		
	}
}
