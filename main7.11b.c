#include "LPC17xx.h"                    // Device header
#include "LCD_ILI9325.h"
#include "Open1768_LCD.h"

int main()
{
       //trzy osobne rejestry
       //ADRX_RAM
       //ADRY_RAm
       //DATA_RAM
	int registerStatus=lcdReadReg(OSCIL_ON);
	
	lcdWriteReg(ADRX_RAM,  0);
	lcdWriteReg(ADRY_RAM,  0);
	lcdWriteIndex(DATA_RAM);
    while(1)
    {
						
            lcdWriteData(LCDBlue);
    }

}
