#include "LPC17xx.h"                    // Device header
#include "LCD_ILI9325.h"
#include "Open1768_LCD.h"
#include "asciiLib.h"
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include <stdlib.h>
#include <cmath>
#include "TP_Open1768.h"

uint32_t active;
static int i=0;
//funkcja przesylajaca cale stringi na uarta
void fun( char *x)
{
        LPC_UART0->FCR=7;
    while(*x!='\0')
    {
				//sprawdzamy czy kolejka jest pusta
        if(LPC_UART0->LSR&(1<<5))
        {
            LPC_UART0->THR=*x;
            x++;
        }
        else{

        }

    }
}

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
				lcdWriteReg(DATA_RAM,LCDBlue);
			}

		}
	
	}
}
void rysujprostokat( uint16_t x, uint16_t y,uint16_t xx, uint16_t yy)
{
	for(uint16_t j=x;j<(x+xx);j++)
	{	
		for(uint16_t i=(y);i<(y+yy);i++)
		{
			
				lcdWriteReg(ADRX_RAM,  i);
				lcdWriteReg(ADRY_RAM,  j);
				lcdWriteReg(DATA_RAM,LCDGreen);
		}
			

	}
}
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
	/*lcdWriteReg(HADRPOS_RAM_START,  0);
	lcdWriteReg(HADRPOS_RAM_END,  100);
	lcdWriteReg(VADRPOS_RAM_START,  0);
	lcdWriteReg(VADRPOS_RAM_END,  100);
	lcdWriteReg(ENTRYM,3);*/
	uint16_t i=0;
   while(i<100)
    {
						
            lcdWriteData(LCDRed);
		i++;
    }
	lcdWriteReg(ADRX_RAM,  100);
	lcdWriteReg(ADRY_RAM,  100);
	lcdWriteIndex(DATA_RAM);
		//tutaj jakbysmy chcieli okno narysowac
	/*lcdWriteReg(HADRPOS_RAM_START,  0);
	lcdWriteReg(HADRPOS_RAM_END,  100);
	lcdWriteReg(VADRPOS_RAM_START,  0);
	lcdWriteReg(VADRPOS_RAM_END,  100);
	lcdWriteReg(ENTRYM,3);*/
	i=0;
   while(i<100)
    {
						
            lcdWriteData(LCDRed);
		i++;
    }
	lcdWriteReg(ADRX_RAM,  200);
	lcdWriteReg(ADRY_RAM,  200);
	lcdWriteIndex(DATA_RAM);
	/*for(uint16_t j=0;j<16;j++)
	{
		
		for(i=0;i<8;i++)
		{
			//lcdWriteReg(ADRX_RAM,  i);
			//lcdWriteReg(ADRY_RAM,  j);
			//lcdWriteReg(DATA_RAM,LCDRed);
			//lcdWriteData(LCDRed);
			unsigned char costam[8];
			GetASCIICode(1,costam,'L');
			if(*(costam+j) & (1<<abs(8-i)))
			{
				lcdWriteReg(ADRX_RAM,  i);
				lcdWriteReg(ADRY_RAM,  j);
				lcdWriteReg(DATA_RAM,LCDRed);
			}
			else{
				lcdWriteReg(ADRX_RAM,  i);
				lcdWriteReg(ADRY_RAM,  j);
				lcdWriteReg(DATA_RAM,LCDBlue);
			}

		}
	}*/
	rysuj('A',0,0);
	rysuj('L',0,8);
	rysuj('A',0,16);
	rysuj(' ',0,24);
	rysuj('M',0,32);
	rysuj('A',0,40);
	rysuj(' ',0,48);
	rysuj('K',0,56);
	rysuj('O',0,64);
	rysuj('T',0,72);
	rysuj('A',0,80);
	for(i=0;i<8;i++)
		rysujprostokat(32+i*32,10,28,130);
		
	PIN_Configure(0,2,1,0,0);
    PIN_Configure(0,3,1,0,0);

    LPC_UART0->LCR=3|(1<<7);
    LPC_UART0->DLL=27;
    LPC_UART0->DLM=0;
    LPC_UART0->LCR=3;

    LPC_UART0->LCR=3;
	touchpanelInit();
	
	
	while(1){
		int x[2];
		touchpanelGetXY(x,x+1);
		
			//for(int i=0;i<4;i++)
				int haha = *x%10;
				char litera = '0'+haha;
				if(haha!=0)fun(&litera);
				//*x/=10;
			
			char * przerwa = "    ";
			fun(przerwa);
			
			//for(int i=0;i<4;i++){
				haha = *(x+1)%10;
				litera = '0'+haha;
				if(haha!=0)
				{	
					fun(&litera);
				}
				//*(x+1)/=10;
			//}
				//char *x="AlA";
				//fun(&x );

		
	}
	//char *x="AlA";
   ///fun(x );


   

}
