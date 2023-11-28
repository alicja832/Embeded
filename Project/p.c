#include "LPC17xx.h"                    // Device header
#include "LCD_ILI9325.h"
#include "Open1768_LCD.h"
#include "asciiLib.h"
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include <stdlib.h>
#include "TP_Open1768.h"

uint32_t zapis=0;
uint32_t active;
volatile uint32_t msTicks = 0;
volatile uint32_t msTicks2 = 0;
void initSPI(void) {
	//chyba trzeba jeszcze skonfigurowac piny
	//serial clock for spi
	PIN_Configure(0,15,3,0,0);
	//master in slave out for spi
	PIN_Configure(0,16,3,0,0);
	//master in slave out for spi
	PIN_Configure(0,17,3,0,0);
	//master out slave in for spi
	PIN_Configure(0,18,3,0,0);
    // Configure SCK, MOSI, and SSEL as per your hardware setup
    // Enable SPI, set as master, and set clock rate
   LPC_SPI->SPCR = (1 << 5) | (1 << 4) | (1 << 3);
   LPC_SPI->SPCCR = 8; // Adjust the clock rate as needed

}
// Function to send a byte via SPI
void sendByteSPI(unsigned char data) {
    LPC_SPI->SPDR = data;
    while (!(LPC_SPI->SPDR & (1 << 7))); // Wait for SPIF flag
}


//tablica do zapisu dzwiekow,wkaznik bedzie wskazywal na tablice
void initDAC(void) {
	//konfiguracja pinow
PIN_Configure(0,26,2,2,0);
    // Enable DAC output
	
   //LPC_DAC->DACCTRL |= (1 << 2);
}

// Function to send data to DAC
void sendToDAC(unsigned short data) {
    LPC_DAC->DACR = (data << 6);
}

//delay na pewno sie przyda
void conf(void)
{
	//dlatego 10 poniewaz chcemy aby umiescic taka wartosc aby zegar wykonywal przerwanie raz na 0,1 s
	SysTick_Config(SystemCoreClock/1000.0);
}
void SysTick_Handler(void)
{   
	msTicks++;
	
}
void TIMER0_IRQHandler(void)  {
    /* Timer 0 interrupt handler  */
   LPC_TIM0->IR |= (1 << 0);
   msTicks2++;
}
void delay2(int d)
{
			msTicks2=0;
			while(d>msTicks2);
}

void delay(int d)
{
			msTicks=0;
			while(d>msTicks);
}
void zagraj()
{
	int i=0;
	while(1){
		    sendToDAC(128);
			delay(1);
			sendToDAC(0);
			delay(1);
        	i++;
		}
}

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
//w tej funkcji bedziemy zapisywac elementy do tablicy jesli flaga zapis bedzie ustalona na 1, innaczej kazdy jeden dzwiek bedzie 
//przesylany do odtwarzania
void jakidzwiek(int xx, int yy)
{
	char res1[10];
	char res2[10];
	char *n=" \n";
	xx=xx-500;

	char *tab[]={"1","2","3","4","5","6","7","8"};
	
	int sk=350;
	int sky=200;
	int k;
	//ustalenie ktory klawisz zostal nacisniety
	for(k=0;k<8;k++)
	{
		if( xx>k*sk && xx<(k+1)*sk)
		{
			fun(tab[k]);
			
		}
	}
	

}
//musi byc oddzielny timer -niezalezny delay
void EINT3_IRQHandler()
{
	int k;
	while(k < 100)k++;
	int x[2];
	char res1[20];
	char res2[20];
	//rzeczy pomagajace cos sprawdzic
	//char *r="\r ";
	//char *n=" \n";
	//char *s=" ";
		//zagraj();
	touchpanelGetXY(x,x+1);
	LPC_GPIOINT->IO0IntClr=(1<<19);
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	jakidzwiek(x[0],x[1]);
	zagraj();
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
	touchpanelInit();
    conf();
	initDAC();
	
	int registerStatus=lcdReadReg(OSCIL_ON);
	
	lcdWriteReg(ADRX_RAM,  0);
	lcdWriteReg(ADRY_RAM,  0);
	lcdWriteIndex(DATA_RAM);
	
		//tutaj jakbysmy chcieli okno narysowac wydajniej
	/*lcdWriteReg(HADRPOS_RAM_START,  0);
	lcdWriteReg(HADRPOS_RAM_END,  100);
	lcdWriteReg(VADRPOS_RAM_START,  0);
	lcdWriteReg(VADRPOS_RAM_END,  100);
	lcdWriteReg(ENTRYM,3);*/
	
	uint16_t i=0;
  
	//lcdWriteReg(ADRX_RAM,  100);
	//lcdWriteReg(ADRY_RAM,  100);
	//lcdWriteIndex(DATA_RAM);
	
  while(i<100)i++; /// idk po co,delay
	
	//konfiguracja GPIO do informowania o przerwaniach-dotykanie ekranu
	 PIN_Configure (0,19,0,0,0);
   LPC_GPIOINT->IO0IntEnF=(1<<19);
   NVIC_EnableIRQ(EINT3_IRQn);
   NVIC_GetActive(EINT3_IRQn) ;
	
	//to chyba mozna usunac!!
	//lcdWriteReg(ADRX_RAM,  200);
	//lcdWriteReg(ADRY_RAM,  200);
	lcdWriteIndex(DATA_RAM);
	
	rysuj('R',0,0);
	rysuj('E',0,8);
	rysuj('C',0,16);
	rysuj(' ',0,24);
	rysuj('P',0,32);
	rysuj('L',0,40);
	rysuj('A',0,48);
	rysuj('Y',0,56);
	
	for(i=0;i<8;i++)
		rysujprostokat(32+i*32,10,28,130);
		
	PIN_Configure(0,2,1,0,0);
    PIN_Configure(0,3,1,0,0);

    LPC_UART0->LCR=3|(1<<7);
    LPC_UART0->DLL=27;
    LPC_UART0->DLM=0;
    LPC_UART0->LCR=3;

    LPC_UART0->LCR=3;
	
	//initSPI();
	zagraj();
	
	
	//nie wiem po co to
	while(1)
    {
		
		//delay(10);
		//zagraj();
		//delay(100);
		
    }
	 

}
