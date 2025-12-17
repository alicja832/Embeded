#include "RTE_Device.h"                 // Keil::Device:Startup
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "LPC17xx.h"  
#include "PIN_LPC17xx.h"                // Keil::Device:PIN// Device header
#include "GPIO_LPC17xx.h"
#include "cmsis_os2.h"
#include "string.h"
#include <stdio.h>
#include "asciiLib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LCD_ILI9325.h"
#include "Open1768_LCD.h"
#include <stdlib.h>

#define CLK_PIN 19
#define DT_PIN 21

extern ARM_DRIVER_USART Driver_USART1;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART1;
bool pressure_flag = false;
bool temperature_flag = false;
bool humidity_flag = false;

void rysuj(char literka, uint16_t x, uint16_t y){
	
	for(uint16_t j=x;j<(x+16);j++)
	{	
		for(uint16_t i=(y);i<(y+8);i++)
		{
			unsigned char costam[1];
			GetASCIICode(1,costam,literka);
			if(*(costam+j-x) & (1<<abs(8-(i-y))))
			{
				lcdWriteReg(ADRX_RAM,  i);
				lcdWriteReg(ADRY_RAM,  j);
				lcdWriteReg(DATA_RAM,LCDRed);
			}
			else{
				lcdWriteReg(ADRX_RAM,  i);
				lcdWriteReg(ADRY_RAM,  j);
				lcdWriteReg(DATA_RAM,LCDYellow);
			}

		}
	
	}
}

void rysujprostokat( uint16_t x, uint16_t y,uint16_t xx, uint16_t yy,uint16_t color)
{
	for(uint16_t j=x;j<(x+xx);j++)
	{	
		for(uint16_t i=(y);i<(y+yy);i++)
		{
			
				lcdWriteReg(ADRX_RAM,  i);
				lcdWriteReg(ADRY_RAM,  j);
				lcdWriteReg(DATA_RAM,color);
		}
			

	}
}


void draw_values(float value)
{
	char values_string[8];


	char values_string[8];
	//np value = 1023,51
    int k;
	for (k=4;k>0;k--){
		values_string[4-k] = (char)( (int)value/(int)pow(10,k-1)%10 + 48);
	}
    values_string[4] = '.';
    float value_float = (value - (int)value);
    for (int j=2;j>0;j--){
		values_string[4+j] = (char)( (int)(value_float/pow(0.1,j))%10 + 48);
	}
	if(humidity_flag)
	{
		// Draw humidity value
		//rysuj(humidity[k],x_start_postion + k*8,y_position);

	}
	if(temperature_flag)
	{
		// Draw temperature value
		//		rysuj(humidity[k],x_start_postion + k*8,y_position);
	
	}
	if(pressure_flag)
	{
		// Draw pressure value
		//		rysuj(humidity[k],x_start_postion + k*8,y_position);

	}
}

void draw_menu(void)
{
	const char humidity[] = {'H','U','M','I','D','I','T','Y'};
	const char temp[] = {'T','E','M','P','E','R','A','T','U','R','E'};
	const char press[] = {'P','R','E','S','S','U','R','E'};
	uint16_t color_tla = LCDWhite;//!!????
	lcdWriteIndex(DATA_RAM);
	int y_position = 32;
	int x_start_postion = 32;
	int k;
	
	if(humidity_flag)
	{
		rysujprostokat(x_start_postion,10,32*8,50,LCDWhite);
		rysujprostokat(x_start_postion,100,32*8,100,LCDWhite);
	}
	else
	{
		rysujprostokat(x_start_postion,10,32*8,50,LCDWhite);
		rysujprostokat(x_start_postion,100,32*8,100,color_tla);

	}
	for (k=0;k<8;k++){
		rysuj(humidity[k],x_start_postion + k*8,y_position);
	}	  
			  
	x_start_postion = x_start_postion + k*8;
	if(temperature_flag)
	{
		rysujprostokat(x_start_postion,10,32*8,50,LCDWhite);
		rysujprostokat(x_start_postion,100,32*8,100,LCDWhite);
	}
	else
	{
		rysujprostokat(x_start_postion,10,32*8,50,LCDRed);
		rysujprostokat(x_start_postion,100,32*8,100,color_tla);
	}
	for (k=0;k<11;k++){
		rysuj(temp[k],x_start_postion + k*8,y_position);
	}
			  
	x_start_postion = x_start_postion + k*8;
	if(pressure_flag)
	{
		rysujprostokat(x_start_postion,10,32*8,50,LCDWhite);
		rysujprostokat(x_start_postion,100,32*8,100,LCDWhite);
	}
	else
	{
		rysujprostokat(x_start_postion,10,32*8,50,LCDRed);
		rysujprostokat(x_start_postion,100,32*8,100,color_tla);
	}
	for (k=0;k<11;k++){
		rysuj(press[k],x_start_postion + k*8,y_position);
	}
}

void encoder_task(void * tmp)
{
	int counter = 0;
	char test[20];
	
			

	    while(1) {
			
		  if (i%100000 == 0) {
			  
			  sprintf(test, "counter: %d\r\n", counter);
			  USARTdrv->Send(test, strlen(test));
		  }
		  //vTaskSuspendAll();
		
		  if (GPIO_PinRead(1, CLK_PIN) == 0)  // If the OUTA is RESET
				{
			if (GPIO_PinRead(1, DT_PIN) == 0)  // If OUTB is also reset... CCK
		  {
			  //while (GPIO_PinRead(1, DT_PIN) == 0){};  // wait for the OUTB to go high
			  counter--;
			  //while (GPIO_PinRead(1, CLK_PIN) == 0){};  // wait for the OUTA to go high
			  for(int i=0; i<10000; i++){
				j++;
			  }  
		  }

		  else  // If OUTB is also set
		  {
			  //while (GPIO_PinRead(1, DT_PIN) == 1){};  // wait for the OUTB to go LOW.. CK
			  counter++;
			  //while (GPIO_PinRead(1, CLK_PIN) == 0){};  // wait for the OUTA to go high
			  //while (GPIO_PinRead(1, DT_PIN) == 0){};  // wait for the OUTB to go high
			  for(int i=0; i<10000; i++){
				j++;
			  }
		  }
		}
		i++;
		//xTaskResumeAll();
	}
}

int main(void)
{
	lcdConfiguration();
	init_ILI9325();
    PIN_Configure (1,CLK_PIN,0,0,0);
	PIN_Configure (1,DT_PIN,0,0,0);

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
	int registerStatus=lcdReadReg(OSCIL_ON);
	
	lcdWriteReg(ADRX_RAM,  0);
	lcdWriteReg(ADRY_RAM,  0);
	lcdWriteIndex(DATA_RAM);
	SystemCoreClockUpdate();
	osKernelInitialize();

	void *thread = osThreadNew(encoder_task,NULL,NULL);
	
	osKernelStart();
	
	while(1)
	{
		
	}
}