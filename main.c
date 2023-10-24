#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_Buttons.h"              // ::Board Support:Buttons
#include "LPC17xx.h"                    // Device header
#include "Board_Joystick.h"             // ::Board Support:Joystick
#include "Driver_USART.h"               // ::CMSIS Driver:USART

#include <stdio.h>
#include <string.h>

#define LED_NUMBER 8

volatile int msTicks = 0;

void SysTick_Handler(void) {
	msTicks++;
}

void delay(int t) {
	msTicks = 0;
	while(t>msTicks);
}

extern ARM_DRIVER_USART Driver_USART1;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART1;
   
char                   cmd;




int main(int argc, char** argv) {
	
	uint32_t retC;
	retC = SysTick_Config(SystemCoreClock/10);
	//int LEDState = 0;
	Buttons_Initialize();
	Joystick_Initialize ();
	LED_Initialize();

	USARTdrv->Initialize(NULL);    
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
									ARM_USART_DATA_BITS_8 |                      
								ARM_USART_PARITY_NONE |                      
								ARM_USART_STOP_BITS_1 |                      
									ARM_USART_FLOW_CONTROL_NONE, 4800);        
USARTdrv->Control (ARM_USART_CONTROL_TX, 1);    
USARTdrv->Control (ARM_USART_CONTROL_RX, 1);     
USARTdrv->Send("\nPress Enter to receive a message", 34);
	
	
	
	for(int i = 0; i<8; i++) {
		LED_Off(i);
	}
	while(1) {
		LED_On(0);
		delay(5);
		LED_Off(0);
		delay(5);
	}
	
	return 0;
}

/*
if(LEDState) LED_On(0);
		else LED_Off(0);
		while(Buttons_GetState()&(1 << 0 )) {
			delay(100);
				if(Buttons_GetState()&(1 << 0 )) {
					if(LEDState) LEDState=0;
					else LEDState = 1;
				}
*/