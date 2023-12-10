#include "LPC17xx.h"                    // Device header
#include "LCD_ILI9325.h"
#include "Open1768_LCD.h"
#include "asciiLib.h"
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include <stdlib.h>
#include "TP_Open1768.h"

uint32_t zapis;
uint32_t active;

volatile uint32_t msTicks2 = 0;

//moj autorski glupi troche pomysl- 
//zastosowanie struktury, zeby mozna bylo tam wsadzic dzwiek oraz do tego czas przez jaki ma byc odgrywany
struct nutka
{
	short int dzwiek;
  int time;
};
//tablica do ktorej bedziemy zapisywac dzwieki
struct nutka melody[100];

void TIMER0_IRQHandler(void){
   /* Timer 0 interrupt handler  */
	 LPC_TIM0->IR |= (1 << 0);
	 msTicks2++;
}

//tablica do zapisu dzwiekow,wkaznik bedzie wskazywal na tablice
void initDAC(void) {
	//konfiguracja pinow,mode open drain
	PIN_Configure(0,26,2,2,0);
    // Enable DAC output
}
//??????????
void initSPI(void) {
	//chyba trzeba jeszcze skonfigurowac piny, when 10- ssel0 miso0 when 11 miso,SCK0,SCK
	//serial clock for spi, czwarta chyba ma byc chyba 2-nor pull- down nor pull-up
	PIN_Configure(0,15,2,2,0);
	//ssel for spi
	PIN_Configure(0,16,2,2,0);
	//master in slave out for spi
	PIN_Configure(0,17,2,2,0);
	//master out slave in for spi
	PIN_Configure(0,18,2,2,0);
    // Configure SCK, MOSI, and SSEL as per your hardware setup
	//trzeba ustawic master mode
	
    // Enable SPI, set as master, and set clock rate,maybe ok
   LPC_SPI->SPCR = (1 << 5) | (1 << 4) | (1 << 2);
   LPC_SPI->SPCCR = 8; // Adjust the clock rate as needed

}
///?????????
// Function to send a byte via SPI
void sendByteSPI(unsigned short data) {
	//. Write the data to transmitted to the SPI Data Register. This write starts the SPI data transfer
    LPC_SPI->SPDR = data;
	//3. Wait for the SPIF bit in the SPI Status Register to be set to 1. The SPIF bit will be set 
	//after the last cycle of the SPI data transfer
    while (!(LPC_SPI->SPDR & (1 << 7))); // Wait for SPIF flag
}
//zegar
void delay2(int d)
{
		msTicks2=0;
		while(d>msTicks2);
}

// Function to send data to DAC
void sendToDAC(unsigned short data) {
    
		LPC_DAC->DACR = (data << 6);

}
//nalezy sprawdzic czy jest to kwestia zle ustawionego timera ze zle gra
void zagraj2(int f,int time)
{
	//jak to powinno byc zrobione:
	int pwm_period=1000/f;//dlaczego tys, tu powinna byc teoretycznie 1 s, ale mi intuicja mowi ze bedzie tu 1000
	//dlugosc trwania
	int t=time/pwm_period;
	
	for(int i=0;i<t;i++)
	{
			sendToDAC(128);
			delay2(pwm_period);
			sendToDAC(0);
			delay2(pwm_period);
			
	}
}
//w tej funkcji bedziemy zapisywac elementy do tablicy jesli flaga zapis bedzie ustalona na 1, innaczej kazdy jeden dzwiek bedzie 
//przesylany do odtwarzania
void jakidzwiek(int xx, int yy)
{
	char *n=" \n";
	xx=xx-500;

	//char *tab[]={"1","2","3","4","5","6","7","8"};
	int f[]={262,294,330,349,392,440,493,523};
	int sk=350;
	//int sky=200;
	short int k;
	//ustalenie ktory klawisz zostal nacisniety
	for(k=0;k<8;k++)
	{
		if( xx>k*sk && xx<(k+1)*sk)//y do dodania
		{
			//fun(tab[k]);
			if(zapis>=0 && zapis<100)
			{
				melody[zapis].dzwiek=f[k];
				melody[zapis].time=1000;
				zapis++;
			}	
			//sprawdzanie czy dziala 	ta funkcja,mozna sprawdzic z poziomu  maina
			delay2(1000);
			zagraj2(f[k],1000);
			delay2(1000);
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

//musi byc oddzielny timer -niezalezny delay
void EINT3_IRQHandler()
{	
	int x[2];
	
	touchpanelGetXY(x,x+1);
	LPC_GPIOINT->IO0IntClr=(1<<19);
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	//trzeba jeszcze dodac funkcjonalnosc, ktora pozwoli na obsluge rec i play
	//jesli rec- zapis=1;
	//co jesli play?
	jakidzwiek(x[0],x[1]);
	//to nie dziala jak powinno
	//if play
	//for(k=0;k<zapis;k++)
	//zagraj2(melody[k].dzwiek,melody[k].time)		
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
int main()
{
	
	LPC_TIM0->PR=SystemCoreClock/1000000-1;
  LPC_TIM0->MR0=1000-1;
  LPC_TIM0->MCR=3;
  NVIC_EnableIRQ(TIMER0_IRQn);
	//wlaczanie timera
  LPC_TIM0->TCR = 1;
  active = NVIC_GetActive(TIMER0_IRQn);
	
	lcdConfiguration();
	init_ILI9325();
	touchpanelInit();
	initDAC();
	initSPI();
	//sendByteSPI(128);
	
	int registerStatus=lcdReadReg(OSCIL_ON);
	
	lcdWriteReg(ADRX_RAM,  0);
	lcdWriteReg(ADRY_RAM,  0);
	lcdWriteIndex(DATA_RAM);
	
	//konfiguracja GPIO do informowania o przerwaniach-dotykanie ekranu
	 PIN_Configure (0,19,0,0,0);
   LPC_GPIOINT->IO0IntEnF=(1<<19);
   NVIC_EnableIRQ(EINT3_IRQn);
   NVIC_SetPriority(EINT3_IRQn,1) ;
   NVIC_GetActive(EINT3_IRQn) ;
  
	
	lcdWriteIndex(DATA_RAM);
	rysujprostokat(0,0,320,240,LCDBlue);
	rysuj('R',0,0);
	rysuj('E',0,8);
	rysuj('C',0,16);
	rysuj(' ',0,24);
	rysuj('P',0,32);
	rysuj('L',0,40);
	rysuj('A',0,48);
	rysuj('Y',0,56);
	
	short int i;
	for(i=0;i<8;i++)
		rysujprostokat(32+i*32,10,28,130,LCDGreen);

	zagraj(440,1000);
	zagraj(240,1000);
	//zeby sprobowac
	zagraj2(240,1000);
	
	//glowna petla programu
	while(1)
    {
		
    }
	 

}
