#include "Board_Buttons.h"              // ::Board Support:Buttons
#include "Board_LED.h"                  // ::Board Support:LED
#include "RTE_Device.h"                 // Keil::Device:Startup
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_Joystick.h"             // ::Board Support:Joystick
#include "LPC17xx.h"                    // Device header

volatile uint32_t msTicks = 0;


void conf(void)
{
	//dlatego 10 poniewaz chcemy aby umiescic taka wartosc aby zegar wykonywal przerwanie raz na 0,1 s
	SysTick_Config(SystemCoreClock/10);
}
void SysTick_Handler(void)
{
	msTicks++;
}

void delay(int d)
{
			msTicks=0;
			while(d>msTicks);
}

//jest zamontowany USART1 na plytce
extern ARM_DRIVER_USART Driver_USART1;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART1;
char cmd;



int main(void)
{
	Buttons_Initialize();
	//wtedy sie wszystkie diody sie swieca
	LED_Initialize();
	//inicjalizacja joysticka
	Joystick_Initialize();
	conf();
	//zmienna do zapalania kolejno lampek
	int num=0;
	
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
 
   USARTdrv->Send("\nPress Enter to receive a message", 34);
    
	while(1)
	{
		//druga lampka miga-zapala sie i wlacza przy kazdej petli
		LED_On(2);
		
		//lampki gasza sie kolejno
		if(Buttons_GetState())
				LED_Off(num);
		else{
			LED_On(num);
			num++;
		}
		switch(Joystick_GetState())
		{
			case(JOYSTICK_DOWN):
			{
				LED_Off(1);
				delay(1);
				LED_On(1);
				break;
				
			}
			case(JOYSTICK_UP):
			{
				LED_Off(2);
				delay(1);
				LED_On(2);
				break;
			}
			case(JOYSTICK_RIGHT):
			{
				LED_Off(3);
				delay(1);
				LED_On(3);
				break;
			}
			case(JOYSTICK_LEFT):
			{
				LED_Off(0);
				delay(1);
				LED_On(0);
				break;
			}
		}
		delay(1);
		LED_Off (2);
		delay(1);
		
		if(num>7)
			num=0;
		
	}
}
