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
void TIMER0_IRQHandler(void)  {
    /* Timer 0 interrupt handler  */
LPC_TIM0->IR |= (1 << 0);
	 msTicks2++;
   // USARTdrv->Send("Ping\n", 4);
}
//procedura inicjujaca uarta
void initUART(void)
{
		
		PIN_Configure(0,2,1,0,0);
    PIN_Configure(0,3,1,0,0);

    LPC_UART0->LCR=3|(1<<7);
    LPC_UART0->DLL=27;
    LPC_UART0->DLM=0;
    LPC_UART0->LCR=3;

    LPC_UART0->LCR=3;
}


//tablica do zapisu dzwiekow,wkaznik bedzie wskazywal na tablice
void initDAC(void) {
	//konfiguracja pinow,mode open drain
PIN_Configure(0,26,2,2,0);
    // Enable DAC output
	
   //LPC_DAC->DACCTRL |= (1 << 2);
}
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
    // Enable SPI, set as master, and set clock rate
   LPC_SPI->SPCR = (1 << 5) | (1 << 4) | (1 << 2);
   LPC_SPI->SPCCR = 8; // Adjust the clock rate as needed

}
// Function to send a byte via SPI
void sendByteSPI(unsigned short data) {
    LPC_SPI->SPDR = data;
	//tutaj nie za bardzo wiem o co chodzi
    while (!(LPC_SPI->SPDR & (1 << 7))); // Wait for SPIF flag
}
void SysTick_Handler(void)
{   
	msTicks++;
	
}
//zegar
void conf(void)
{
	//dlatego 10 poniewaz chcemy aby umiescic taka wartosc aby zegar wykonywal przerwanie raz na 0,001 s
	SysTick_Config(SystemCoreClock/1000.0);
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
// Function to send data to DAC
void sendToDAC(unsigned short data) {
    LPC_DAC->DACR = (data << 6);
}


//delay na pewno sie przyda

void zagraj(int f,int time)
{
	
	//jak to powinno byc zrobione:
	
	int pwm_period=1000/f;//dlaczego tys, tu powinna byc teoretycznie 1 s, ale mi intuicja mowi ze bedzie tu 1000
	//dlugosc trwania
	int t=time/pwm_period;
	for(int i=0;i<t;i++)
	{
			sendToDAC(128);
			delay(pwm_period);
			sendToDAC(0);
			delay(pwm_period);
			
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
	int f[]={262,294,330,349,392,440,493,523};
	int sk=350;
	int sky=200;
	int k;
	//ustalenie ktory klawisz zostal nacisniety
	for(k=0;k<8;k++)
	{
		if( xx>k*sk && xx<(k+1)*sk)//y do dodania
		{
			fun(tab[k]);
			//sprawdzanie czy dziala ta funkcja,mozna sprawdzic z poziomu  maina
			delay(1000);
			zagraj(f[k],1000);
			delay(1000);
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
	//int k;
	//while(k < 100)k++;
	int x[2];
	//char res1[20];
	//char res2[20];
	touchpanelGetXY(x,x+1);
	LPC_GPIOINT->IO0IntClr=(1<<19);
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	//delay2(1);
	//zagraj(240,1000);
	//while(i<1000) i++;
	
	//jakidzwiek(x[0],x[1]);
	//to nie dziala jak powinno
	{
	char *tab[]={"1","2","3","4","5","6","7","8"};
	short int f[]={262,294,330,349,392,440,493,523};
	//int sk=350;
	//int sky=200;
	short int k;
	//ustalenie ktory klawisz zostal nacisniety
	x[0]=x[0]-500;
	for(k=0;k<8;k++)
	{
		if( x[0]>k*350 && x[0]<(k+1)*350)//y do dodania
		{
			int pwm_period=1000/f[k];
			for(short int i=0;i<100/pwm_period;i++)
			{
			sendToDAC(128);
			delay2( pwm_period);
			sendToDAC(0);
			delay2( pwm_period);
			
			}
		}
	}
	//short int f=440;
	//int time=10000;
	/*int pwm_period=1000/440;//dlaczego tys, tu powinna byc teoretycznie 1 s, ale mi intuicja mowi ze bedzie tu 1000
	//dlugosc trwania
	//short int t=10;
	for(short int i=0;i<100/pwm_period;i++)
	{
			sendToDAC(128);
			delay2( pwm_period);
			sendToDAC(0);
			delay2( pwm_period);
			
	}*/
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
int main()
{
    //LPC_TIM0->PR=SystemCoreClock/1000000-1;
    //LPC_TIM0->MR0=1000-1;
    //LPC_TIM0->MCR=3;
    LPC_TIM0->PR=250000-1;
    LPC_TIM0->MR0=1;
    LPC_TIM0->MCR=3;
    NVIC_EnableIRQ(TIMER0_IRQn);
		//wlaczanie timera
    LPC_TIM0->TCR = 1;
    active = NVIC_GetActive(TIMER0_IRQn);
	lcdConfiguration();
	init_ILI9325();
	touchpanelInit();
	conf();
	initDAC();
	initSPI();
	//sendByteSPI(1);
	//initUART();
	
	int registerStatus=lcdReadReg(OSCIL_ON);
	
	lcdWriteReg(ADRX_RAM,  0);
	lcdWriteReg(ADRY_RAM,  0);
	lcdWriteIndex(DATA_RAM);
	
	uint16_t i=0;
	
  while(i<100)i++; /// idk po co,delay
	
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
	
	for(i=0;i<8;i++)
		rysujprostokat(32+i*32,10,28,130,LCDGreen);
		
	
	//delay(1000);

	zagraj(440,1000);
	zagraj(240,1000);
	zagraj2(440,1000);
	//glowna petla programu
	while(1)
    {
		
    }
	 

}
