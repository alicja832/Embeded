#include "Driver_USART.h"              
#include "RTE_Device.h"            
#include "LPC17xx.h"                    
#include "Board_LED.h"               
#include "PIN_LPC17xx.h"              

extern ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART0;

uint32_t active;
//Timer 0/1/2/3 Basic Configuration,Dokumentacja Startupu->Reference->Interrupts and Exceptions
void TIMER0_IRQHandler(void)  {
    /* Timer 0 interrupt handler  */
LPC_TIM0->IR |= (1 << 0);
    USARTdrv->Send("Ping\n", 4);
}

void RTC_IRQHandler(void)  {

    LPC_RTC->ILR=1;
    if(active%2==0)
        USARTdrv->Send("Tak \n",4);
    else
        USARTdrv->Send("Tik \n",4);
}
void EINT0_IRQHandler(void)
{
    LPC_SC->EXTINT=1;
    USARTdrv->Send("Click\n",5);
    LED_On (1);
}


int main()
{
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
    LPC_RTC->CCR=17;
    LPC_RTC->CIIR=1;
    NVIC_EnableIRQ(RTC_IRQn);
    NVIC_GetActive(RTC_IRQn) ;

    LED_Initialize();
		//ustalamy jako przerwanie zewnetrzne nacisniecie przyscisku KEY 2
    PIN_Configure (2,10,1,0,0);

    LPC_SC->EXTINT=1;

    LPC_SC->EXTMODE=1;
    LPC_SC->EXTPOLAR=1;
    NVIC_EnableIRQ(EINT0_IRQn);
    NVIC_GetActive(EINT0_IRQn);


    while(1);
	}