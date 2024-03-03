#include "LPC17xx.h"                    // Device header
#include "LCD_ILI9325.h"
#include "Open1768_LCD.h"

int main()
{
	lcdConfiguration();
	init_ILI9325();
       //trzy osobne rejestry
       //ADRX_RAM
       //ADRY_RAm
       //DATA_RAM
	int registerStatus=lcdReadReg(OSCIL_ON);
	
	lcdWriteReg(ADRX_RAM,  0);
	lcdWriteReg(ADRY_RAM,  0);
	lcdWriteIndex(DATA_RAM);
		//tutaj jakbysmy chcieli okno narysowac
	lcdWriteReg(HADRPOS_RAM_START,  0);
	lcdWriteReg(HADRPOS_RAM_END,  100);
	lcdWriteReg(VADRPOS_RAM_START,  0);
	lcdWriteReg(VADRPOS_RAM_END,  100);
	lcdWriteReg(ENTRYM,3);
    while(1)
    {
						
            lcdWriteData(LCDBlue);
    }

}