#include "RTE_Device.h"                 // Keil::Device:Startup
#include "LPC17xx.h"  
#include "PIN_LPC17xx.h"                // Keil::Device:PIN// Device header
#include "GPIO_LPC17xx.h"
#include "cmsis_os2.h"
#include "asciiLib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LCD_ILI9325.h"
#include "Open1768_LCD.h"
#include <stdlib.h>
#include "Driver_I2C.h"

#define CLK_PIN 19
#define DT_PIN 21
#define SW_PIN 19

float temperature;
float pressure;
float humidity;
uint8_t start_new_value = 10;
uint8_t chosen_option = 10;
bool if_option_is_chosen = false;
int32_t tRaw, pRaw, hRaw;
extern ARM_DRIVER_I2C            Driver_I2C0;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C0;
//LCD
void draw_rectangle( uint16_t x, uint16_t y,uint16_t width, uint16_t  height,uint16_t color)
{
	
	for(uint16_t j=x;j<(x+width);j++)
	{	
		for(uint16_t i=(y);i<(y+height);i++)
		{
			
				lcdWriteReg(ADRX_RAM,  i);
				lcdWriteReg(ADRY_RAM,  j);
				lcdWriteReg(DATA_RAM,color);
		}
	}
}

void draw_letter(char literka, uint16_t x, uint16_t y){
	
	unsigned char costam[17];
	GetASCIICode(1,costam,literka);
	for(uint16_t j=x;j<(x+16);j++)
	{	
		for(uint16_t i=(y);i<(y+9);i++)
		{
			
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
void draw_value(float value, int x, int y)
{
	char values_string[7];
	static const int pow[] = {1,10,100,1000};
	static const float pow01[] = {1.0, 0.1, 0.01,0.001};
    int k;
	
	  for (k=4;k>0;k--){
			values_string[4-k] = (char)(((int)value/(int)pow[k-1])%10 + 48);
	  }
    values_string[4] = '.';
    float value_float = (value - (int)value);
    for (int j=2;j>0;j--){
		values_string[4+j] = (char)( (int)(value_float/pow01[j])%10 + 48);
	}
	k=0;
	while(values_string[k]=='0' )
		k+=1;
	
	for(uint8_t i=k;i<7;i++)
		draw_letter(values_string[i],x,y+9*i);
}



/* Returns temperature in DegC, resolution is 0.01 DegC. Output value of ?5123? equals 51.23 DegC.
   t_fine carries fine temperature as global value
*/

//I2C
#define BME280_ADDRESS_v2 0x76
static volatile uint8_t event_outside;
static volatile bool transfer_complete = false;
static volatile bool nack = false;

uint16_t dig_T1,  \
         dig_P1, \
         dig_H1, dig_H3;

int16_t  dig_T2, dig_T3, \
         dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, \
		 dig_H2,  dig_H4, dig_H5, dig_H6;
int32_t t_fine;

static int32_t BME280_compensate_T_int32(int32_t adc_T)
{
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1)))>> 12) *((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}
static uint32_t bme280_compensate_H_int32(int32_t adc_H)
{
	int32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) *\
			v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r *\
					((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) +\
							((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) +\
					8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *\
			((int32_t)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (uint32_t)(v_x1_u32r>>12);
}

static int TrimRead(void)
{
	uint8_t readAddress;
	uint8_t trimdata[32];

	// Read NVM from 0x88 to 0xA1 (25 bytes)
	readAddress = 0x88;
	I2Cdrv->MasterTransmit(BME280_ADDRESS_v2, &readAddress, 1, true);
	while (I2Cdrv->GetStatus().busy);
	if (event_outside != 0x01) {
		return -1;
	}

	I2Cdrv->MasterReceive(BME280_ADDRESS_v2, trimdata, 25, false);
	while (I2Cdrv->GetStatus().busy);
	if (event_outside != 0x01) {
		return -1;
	}

	// Read NVM from 0xE1 to 0xE7 (7 bytes)
	readAddress = 0xE1;
	I2Cdrv->MasterTransmit(BME280_ADDRESS_v2, &readAddress, 1, true);
	while (I2Cdrv->GetStatus().busy);
	if (event_outside != 0x01) {
		return -1;
	}

	I2Cdrv->MasterReceive(BME280_ADDRESS_v2, trimdata + 25, 7, false);
	while (I2Cdrv->GetStatus().busy);
	if (event_outside != 0x01) {
		return -1;
	}

	// Temperature coefficients
	dig_T1 = (uint16_t)(trimdata[1] << 8 | trimdata[0]);
	dig_T2 = (int16_t)(trimdata[3] << 8 | trimdata[2]);
	dig_T3 = (int16_t)(trimdata[5] << 8 | trimdata[4]);

	// Pressure coefficients
	dig_P1 = (uint16_t)(trimdata[7] << 8 | trimdata[6]);
	dig_P2 = (int16_t)(trimdata[9] << 8 | trimdata[8]);
	dig_P3 = (int16_t)(trimdata[11] << 8 | trimdata[10]);
	dig_P4 = (int16_t)(trimdata[13] << 8 | trimdata[12]);
	dig_P5 = (int16_t)(trimdata[15] << 8 | trimdata[14]);
	dig_P6 = (int16_t)(trimdata[17] << 8 | trimdata[16]);
	dig_P7 = (int16_t)(trimdata[19] << 8 | trimdata[18]);
	dig_P8 = (int16_t)(trimdata[21] << 8 | trimdata[20]);
	dig_P9 = (int16_t)(trimdata[23] << 8 | trimdata[22]);

	// Humidity coefficients
	dig_H1 = trimdata[24];
	dig_H2 = (int16_t)(trimdata[26] << 8 | trimdata[25]);
	dig_H3 = trimdata[27];
	dig_H4 = (int16_t)((trimdata[28] << 4) | (trimdata[29] & 0x0F));
	dig_H5 = (int16_t)((trimdata[30] << 4) | (trimdata[29] >> 4));
	dig_H6 = (int8_t)trimdata[31];

	return 0; //Success
}

void I2C_Callback(uint32_t event) {
	transfer_complete = false;
	nack = false;
	
	event_outside = event;
	
	if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
		transfer_complete = true;
	} else if (event & ARM_I2C_EVENT_ADDRESS_NACK) {
		nack = true;
	}
}


int BME280_Config (uint8_t osrs_t, uint8_t osrs_p, uint8_t osrs_h, uint8_t mode, uint8_t t_sb, uint8_t filter)
{
	uint16_t RESET_REG=0xE0;
	uint16_t CTRL_HUM_REG=0xF2;
	uint16_t CONFIG_REG=0xF5;
	uint16_t CTRL_MEAS_REG=0xF4;
	I2Cdrv->Initialize (I2C_Callback);
	 /* Power-on I2C peripheral */  
	I2Cdrv->PowerControl(ARM_POWER_FULL);   
	I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST_PLUS  );
	I2Cdrv->Control(ARM_I2C_OWN_ADDRESS, 0xBC);
	
	
	TrimRead();

	uint8_t datatowrite[2];

	// Reset the device
	datatowrite[0] = RESET_REG;
	datatowrite[1] = 0xB6;  // reset sequence
	I2Cdrv->MasterTransmit(BME280_ADDRESS_v2, datatowrite, 2, false);
	
	while (I2Cdrv->GetStatus().busy);
	
	

	// write the humidity oversampling to 0xF2
	datatowrite[0] = CTRL_HUM_REG;
	datatowrite[1] = osrs_h;
	I2Cdrv->MasterTransmit(BME280_ADDRESS_v2, datatowrite, 2, false);
	while (I2Cdrv->GetStatus().busy);
	
	
	// write the standby time and IIR filter coeff to 0xF5
	datatowrite[0] = CONFIG_REG;
	datatowrite[1] = (t_sb <<5) | (filter << 2) | 0x0000;
	
	I2Cdrv->MasterTransmit(BME280_ADDRESS_v2, datatowrite, 2, false);
	while (I2Cdrv->GetStatus().busy);
	
	
	// write the pressure and temp oversampling along with mode to 0xF4
	datatowrite[0] = CTRL_MEAS_REG;
	datatowrite[1] = (osrs_t <<5) |(osrs_p << 2) | mode;
	I2Cdrv->MasterTransmit(BME280_ADDRESS_v2, datatowrite, 2, false);
	while (I2Cdrv->GetStatus().busy);
	
	
	return 0;  //Success
}
static int BMEReadRaw(void)
{
	uint8_t RawData[8];
	uint8_t readAddress;
	uint8_t ID_REG = 0xD0;
	uint8_t chipID; 
	readAddress = ID_REG;
	uint8_t PRESS_MSB_REG=0xF7;
	
	I2Cdrv->MasterTransmit(0x76, &readAddress, 1, true);
	while (I2Cdrv->GetStatus().busy);
	if (event_outside != 0x01) {
		return -1;
	}
	
	I2Cdrv->MasterReceive(0x76, &chipID, 1, false);
    while (I2Cdrv->GetStatus().busy);
	if (event_outside != 0x01) {
		return -1;
	}
	
	if (chipID == 0x60) {
	
		readAddress = PRESS_MSB_REG;
		
		I2Cdrv->MasterTransmit(0x76, &readAddress, 1, false);
		while (I2Cdrv->GetStatus().busy);
		if (event_outside != 0x01) {
			return -1;
		}
		
		I2Cdrv->MasterReceive(0x76, RawData, 8, false);
		while (I2Cdrv->GetStatus().busy);
		if (event_outside != 0x01) {
			return -1;
		}

		/* Calculate the Raw data for the parameters
		 * Here the Pressure and Temperature are in 20 bit format and humidity in 16 bit format
		 */
		pRaw = (RawData[0]<<12)|(RawData[1]<<4)|(RawData[2]>>4);
		tRaw = (RawData[3]<<12)|(RawData[4]<<4)|(RawData[5]>>4);
		hRaw = (RawData[6]<<8)|(RawData[7]);

		return 0;
	}
	
	return 2;
}   
static uint32_t BME280_compensate_P_int32(int32_t adc_P)
{
	int32_t var1, var2;
	uint32_t p;
	var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig_P6);
	var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
	var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
	var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) *var1)>>1))>>18;
	var1 =((((32768+var1))*((int32_t)dig_P1))>>15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((uint32_t)var1);
	}
	else
	{
		p = (p / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)dig_P8))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
	return p;
}
void BME280_Measure(float *temperature, float *pressure, float *humidity)
{
	
    if (BMEReadRaw() == 0)
    {
        if (tRaw == 0x800000) *temperature = 1012; // temp disabled
        else
            *temperature = (BME280_compensate_T_int32(tRaw)) / 100.0f;

        if (pRaw == 0x800000) *pressure = 1012; // pressure disabled
        else
        {
            *pressure = (BME280_compensate_P_int32(pRaw));  // in Pa
        }

        if (hRaw == 0x8000) *humidity = 1012; // humidity disabled
        else
            *humidity = (bme280_compensate_H_int32(hRaw)) / 1024.0f;
    }
    else
    {
        // if the device is detached
        *temperature = 1010;
        *pressure    = 1010;
        *humidity    = 1010;
    }
}

void draw_humidity(uint16_t color)
{
	const uint8_t width = 45;
	const uint8_t height = 200;
	const uint8_t x_start_postion = 32;	
  const char humidity[] = {'H','U','M','I','D','I','T','Y'};
	uint8_t k;
	draw_rectangle(x_start_postion,10,width,height,color);
	for (k=0;k<8;k++){
			draw_letter(humidity[k],x_start_postion,8*(k+1));
	}
}
void draw_temp(uint16_t color)
{
	uint8_t k;
	const uint8_t width = 45;
	const uint8_t height = 200;
	const uint8_t x_start_postion = 32;	
	const char temp[] = {'T','E','M','P','E','R','A','T','U','R','E'};
	draw_rectangle(x_start_postion+50,10,width,height,color);
		for (k=0;k<11;k++){
			draw_letter(temp[k],x_start_postion+50,8*(k+1));
		}	
}
void draw_pressure(uint16_t color)
{
	uint8_t k;
	const uint8_t width = 45;
	const uint8_t height = 200;
	const uint8_t x_start_postion = 32;	
	const char press[] = {'P','R','E','S','S','U','R','E'};
	draw_rectangle(x_start_postion+100,10,width,height,color);
	for (k=0;k<8;k++){
			draw_letter(press[k],x_start_postion+100,8*(k+1));
	}	
}
void draw_menu()
{
	draw_humidity(LCDWhite);	
	draw_temp(LCDWhite);
	draw_pressure(LCDWhite);
}
void bme280_task(void* tmp) {
		
	while(1) {
		BME280_Measure(&temperature, &pressure, &humidity);
		if(if_option_is_chosen)
		{	
				start_new_value = chosen_option;
				if_option_is_chosen = false;
				draw_menu();
			  osDelay(1000);
		}
		if(start_new_value == 0)
		{
			draw_value(humidity, 50,50);
		}
		if(start_new_value == 1)
		{	
			draw_value(temperature, 100,50);   
		}
		if(start_new_value == 2)
		{	
			draw_value(pressure, 150,50);
		}
		osDelay(1000);	
	}
}

// joystick
void encoder_task(void * tmp)
{
	int counter = 0;
	uint8_t i = 0;
	uint8_t j = 0;
	
	while(1) {
			
		  if (i%1000 == 0) {
			  i=0;
			  uint8_t counter_temp = abs(counter)%12;
			  if(0<counter_temp && counter_temp <=4 )
			  {
					if(chosen_option!=0)
					{	
						draw_humidity(LCDBlueSea);
						draw_temp(LCDWhite);
						draw_pressure(LCDWhite);
						chosen_option=0;
					}
				}
			  else if(4<counter_temp && counter_temp<=2*4)
			  { 
					if(chosen_option!=1)
					{	
					  draw_temp(LCDBlueSea);
						draw_humidity(LCDWhite);
						draw_pressure(LCDWhite);
						chosen_option=1;
				  }
		
			  }
			  else if(8<counter_temp && counter_temp<12)
			  { 
					if(chosen_option!=2)
					{
						draw_temp(LCDWhite);
						draw_humidity(LCDWhite);
						draw_pressure(LCDBlueSea);
						chosen_option=2;
					}
			  }
		  }

		  
		  if (GPIO_PinRead(1, CLK_PIN) == 0)  // If the OUTA is RESET
			{
				if (GPIO_PinRead(1, DT_PIN) == 0)  // If OUTB is also reset... CCK
				{
					while (GPIO_PinRead(1, DT_PIN) == 0){};  // wait for the OUTB to go high
					counter--;
					while (GPIO_PinRead(1, CLK_PIN) == 0){};  // wait for the OUTA to go high
					for(int i=0; i<1000; i++){
						j++;
					}  
			}

		  else  // If OUTB is also set
		  {
			  while (GPIO_PinRead(1, DT_PIN) == 1){};  // wait for the OUTB to go LOW.. CK
			  counter++;
			  while (GPIO_PinRead(1, CLK_PIN) == 0){};  // wait for the OUTA to go high
			  while (GPIO_PinRead(1, DT_PIN) == 0){};  // wait for the OUTB to go high
			  for(int i=0; i<1000; i++){
				j++;
			  }
		  }
		}
		i++;
		
	}
}

void EINT3_IRQHandler()
{
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	LPC_GPIOINT->IO0IntClr=(1<<19);
	if_option_is_chosen = true;
}
int main(void)
{
	lcdConfiguration();
	init_ILI9325();
	SystemCoreClockUpdate();
	osKernelInitialize();
	PIN_Configure (1,CLK_PIN,0,0,0);
	PIN_Configure (1,DT_PIN,0,0,0);
	PIN_Configure (0,SW_PIN,0,0,0);
	LPC_GPIOINT->IO0IntEnF=(1<<19);
	NVIC_EnableIRQ(EINT3_IRQn);
  NVIC_SetPriority(EINT3_IRQn,1) ;
  NVIC_GetActive(EINT3_IRQn);
	draw_menu();
	
	BME280_Config(0x02, 0x05,0x01,0x03,0x00,0x04);
	osThreadNew(encoder_task,NULL,NULL);
	osThreadNew(bme280_task,NULL,NULL);
	osKernelStart();
	
	while(1)
	{
		
	}
}

