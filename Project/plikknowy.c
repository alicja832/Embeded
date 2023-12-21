#include "LPC17xx.h"                    // Device header
#include "LCD_ILI9325.h"
#include "Open1768_LCD.h"
#include "asciiLib.h"
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include <stdlib.h>
#include "TP_Open1768.h"
#include "Driver_SPI.h"                 // ::CMSIS Driver:SPI

uint32_t zapis=0;
uint32_t active;
extern ARM_DRIVER_SPI Driver_SPI2;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI2;
struct nutka
{
	int dzwiek;
  	int time;
	//jak bedzie spi to bedzie uint8_t
};
//tablica do ktorej bedziemy zapisywac dzwieki
struct nutka melody[100];

const uint8_t testdata_out_0[1] = {0};
const uint8_t testdata_out[1] = {255}; 
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
	SPIdrv->Initialize(NULL);
	SPIdrv->PowerControl(ARM_POWER_FULL);
	SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_SW | ARM_SPI_DATA_BITS(8),
	10000000);
	SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);  

}
// Function to send a byte via SPI
void sendByteSPI(uint8_t info) {
	
	//wysylamy jakies 128 albo 255,ja bym dala 255
	//aktywacja
	//uwaga - musimy wyslac infromacje - czyli CONTROL BITS, dlatego 16-bo robimy PDB,wylaczamy drugi konwerter,nie wiem co z tym bitem wskazujacym na napiecie
	uint8_t data=(1<<5);
	
	SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);        /* Transmit some data */  
	SPIdrv->Send(&data,sizeof(data));
	ARM_SPI_STATUS state = SPIdrv->GetStatus();
	
	do {
    state = SPIdrv->GetStatus();
    } while(state.busy == 1);
	//while(SPIdrv->GetStatus().busy==1);
	//teraz dopiero wlasciwe informacje
	     /* Transmit some data */  
	if(info)
		SPIdrv->Send(testdata_out,sizeof(testdata_out));
	else
		SPIdrv->Send(testdata_out_0,sizeof(testdata_out_0));
	state = SPIdrv->GetStatus();
    do {
    state = SPIdrv->GetStatus();
    } while(state.busy == 1);
	//dezaktywacja
		SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
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
void zagraj2(int f,int time)
{
	
	//jak to powinno byc zrobione:
	//sprobujemy to wywolac
	int pwm_period=100000/f;
	//dlugosc trwania
	int t=time/pwm_period;
	for(int i=0;i<t;i++)
	{
			sendByteSPI(1);
			delay2(pwm_period);
			sendByteSPI(0);
			delay2(pwm_period);
			
	}
}

void zagraj(int f,int time)
{
	
	//jak to powinno byc zrobione:
	
	double pwm_period=1./f;//dlaczego tys, tu powinna byc teoretycznie 1 s, ale mi intuicja mowi ze bedzie tu 1000
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
	
	//to nie dziala
	/*if( LPC_GPIOINT->IO0IntStatR & (1<<19))
	{	
		//odliczanie czasu przez jaki przycisk bedzie przycisniety
		//msTicks2=0;
		//LPC_GPIOINT->IO0IntClr=(1<<19);
		//NVIC_ClearPendingIRQ(EINT3_IRQn);
		return;
	}*/
	int x[2];
	
	touchpanelGetXY(x,x+1);
	LPC_GPIOINT->IO0IntClr=(1<<19);
	NVIC_ClearPendingIRQ(EINT3_IRQn);

	//jakidzwiek(x[0],x[1]);
	//to nie dziala jak powinno
	
		//char *tab[]={"1","2","3","4","5","6","7","8"};
		short int f[]={200,294,330,349,392,440,493,523};
		short int k;
		int pwm_period;
		//ustalenie ktory klawisz zostal nacisniety
		//kalibracja
		x[0]=x[0]-500;
		//sprawdzimy czy klawisz dziala grajac inna czestotliwosc
		if(x[0]>(8*350)) 
		{
		  sendByteSPI(1);
			delay2(1000/600);
			sendByteSPI(0);
			delay2(1000/600);
		
			/*for(k=0;k<zapis;k++)
			{
				pwm_period=1000/melody[k].dzwiek;
				sendByteSPI(1);
				delay2( pwm_period);
				sendByteSPI(0);
				delay2( pwm_period);
			}
			zapis=0;*/
			return;
		}
		if(x[0]<(0)) 
		{
			zapis=1;
			//for test
			sendByteSPI(1);
			delay2(1000/700);
			sendByteSPI(0);
			delay2(1000/700);
		}
	
		for(k=0;k<8;k++)
		{
			if( x[0]>k*350 && x[0]<(k+1)*350)//y do dodania
			{
				//kiedy uzyjemy drugiej konfiguracji timera to trzeba wpisac 100000 
				//i wtedy bedzie lepsze odroznienie dzwiekow
				pwm_period=1000/f[k];
				for(short int i=0;i<100/pwm_period;i++)
				{
					sendByteSPI(1);
					delay2( pwm_period);
					sendByteSPI(0);
					delay2( pwm_period);
					if(zapis>0 && zapis<100)
					{
						melody[zapis-1].dzwiek=f[k];
						//narazie tutaj powinien byc czas jak dlugo przycisk byl przycisniety
						melody[zapis-1].time=1000;
						zapis++;
					}	
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
void init_GPIO()
{
   PIN_Configure (0,19,0,0,0);
   LPC_GPIOINT->IO0IntEnF=(1<<19);
   //LPC_GPIOINT->IO0IntEnR=(1<<19);
   NVIC_EnableIRQ(EINT3_IRQn);
   NVIC_SetPriority(EINT3_IRQn,1) ;
   NVIC_GetActive(EINT3_IRQn) ;

}
void initTimer0()
{
	//co jedna milisekunde
	 LPC_TIM0->PR=SystemCoreClock/1000000-1;
   LPC_TIM0->MR0=1000-1;
   LPC_TIM0->MCR=3;
   NVIC_EnableIRQ(TIMER0_IRQn);
	 //wlaczanie timera
	 LPC_TIM0->TCR = 1;
   active = NVIC_GetActive(TIMER0_IRQn);
}
void initTimer0_2()
{
	//co 10 mikrosekund
	 LPC_TIM0->PR=SystemCoreClock/1000000-1;
   LPC_TIM0->MR0=10-1;
   LPC_TIM0->MCR=3;
   NVIC_EnableIRQ(TIMER0_IRQn);
	 //wlaczanie timera
	 LPC_TIM0->TCR = 1;
   active = NVIC_GetActive(TIMER0_IRQn);
}
int main()
{
	initTimer0();
	lcdConfiguration();
	init_ILI9325();
	touchpanelInit();
	conf();
	initDAC();
	initSPI();
	init_GPIO();
	
	
	int registerStatus=lcdReadReg(OSCIL_ON);
	
	lcdWriteReg(ADRX_RAM,  0);
	lcdWriteReg(ADRY_RAM,  0);
	lcdWriteIndex(DATA_RAM);
	
	uint16_t i=0;
	
  while(i<100)i++; /// idk po co,delay
	
	lcdWriteIndex(DATA_RAM);
	rysujprostokat(0,0,320,240,LCDBlue);
	rysuj('R',0,0);
	rysuj('E',0,8);
	rysuj('C',0,16);
	rysuj('P',300,32);
	rysuj('L',300,40);
	rysuj('A',300,48);
	rysuj('Y',300,56);
	
	for(i=0;i<8;i++)
		rysujprostokat(32+i*32,10,28,130,LCDGreen);
	
	//pr�ba
	zagraj2(440,100000);
	
	while(1)
    {
		
    }
	 

}
