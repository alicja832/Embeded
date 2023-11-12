#include "LPC17xx.h"                    // Device header
#include "LCD_ILI9325.h"
#include "Open1768_LCD.h"

int main()
{
	     lcdConfiguration();

		//first solution
    while(1)
    {
						lcdWriteReg(ADRX_RAM,  20);
						lcdWriteReg(ADRY_RAM,  20);
            lcdWriteReg(DATA_RAM,  LCDBlue);
    }

}
