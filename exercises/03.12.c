#include "RTE_Device.h"                 // Keil::Device:Startup
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "LPC17xx.h"                    // Device header
#include "cmsis_os2.h"
#include "string.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include <stdio.h>
#include "FreeRTOS.h"

#include <cmath>

#include "Driver_I2C.h"

#define AHT_ADDRESS 0x38<<1

extern ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART0;

extern ARM_DRIVER_I2C            Driver_I2C0;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C0;

typedef struct {
	float humidity;
	float temperature;
} aht20;

aht20 aht;

void check_value(int returnValue)
{
	char test2[5] = {'!','o', '\0'};
	if(returnValue<0)
	{
		USARTdrv->Send(test2, strlen(test2));
	}
}

int AHT20_Initialize() {
	int returnValue;
	returnValue = I2Cdrv->Initialize (NULL);
	 /* Power-on I2C peripheral */  
	I2Cdrv->PowerControl(ARM_POWER_FULL);   
	/* Configure I2C bus */  
	I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
	I2Cdrv->Control(ARM_I2C_OWN_ADDRESS, AHT_ADDRESS);
	if (returnValue != 0) {
		return returnValue;
	}
	osDelay(1000);
	uint8_t commands[3] = {0xbe, 0x08, 0x00};
	
	returnValue = I2Cdrv->MasterTransmit (AHT_ADDRESS , commands, 3, true);
	check_value(returnValue);
	osDelay(10);
	return returnValue;
}

int AHT20_Make_Measurement() {
	uint8_t aht_buffer[7] ;
	uint8_t commands[3] = {0xAC, 0x33, 0x00};
	int returnValue;

	returnValue = I2Cdrv->MasterTransmit(AHT_ADDRESS, commands, 3, true);
	
	if (returnValue != 0) {
		return returnValue;
	}
	
	osDelay(1000);
	//tu jest problem
	returnValue = I2Cdrv->MasterReceive(AHT_ADDRESS, aht_buffer, 7, true);
	char test2[5] = {'!','g', '\0'};
	if(returnValue<0)
	{
		USARTdrv->Send(test2, strlen(test2));
	}
	else{
		char test3[5] = {'!','h','\n', '\0'};
		USARTdrv->Send(test3, strlen(test3));
	}
	uint32_t hum_data = (aht_buffer[1]<<16)|(aht_buffer[2]<<8)|aht_buffer[3];
	hum_data = hum_data>>4;
	aht.humidity = (float) ((hum_data / pow(2, 20)) * 100);

	uint32_t temp_data = (aht_buffer[3]<<16)|(aht_buffer[4]<<8)|aht_buffer[5];
	temp_data = temp_data & 0xFFFFF;
	aht.temperature = (float) (((temp_data / pow(2, 20)) * 200) - 50);

	return returnValue;
}

void measure_task(void * tmp)
{
	AHT20_Initialize();
	char test2[100];
	
	while(1) {
		AHT20_Make_Measurement();
		
		sprintf(test2, "humidity: %d, temperature: %d\r\n", (int)aht.humidity, (int)aht.temperature);
		USARTdrv->Send(test2, strlen(test2));
		
		osDelay(5000);
	}
}

void app_main(void * tmp)
{
	while(1) {
		LED_On(1);
		osDelay(1000);
		LED_Off(1);
		osDelay(1000);
	}
}

int main(void)
{
	LED_Initialize();
	
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
	
	SystemCoreClockUpdate();
	osKernelInitialize();

	void *thread = osThreadNew(measure_task,NULL,NULL);
	
	osKernelStart();
	while(1)
	{
		
	}
}
