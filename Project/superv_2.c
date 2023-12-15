#include "LPC17xx.h"                    // Device header
#include "LCD_ILI9325.h"
#include "Open1768_LCD.h"
#include "asciiLib.h"
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include <stdlib.h>
#include "TP_Open1768.h"
#include "Driver_SPI.h"                 // ::CMSIS Driver:SPI

//zmienna globalna informujaca czy zapis jest wlaczony
uint32_t zapis=-1;
uint32_t active;

//rzeczy do SPI
extern ARM_DRIVER_SPI Driver_SPI2;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI2;
const uint8_t testdata_out_0[1] = {0};
const uint8_t testdata_out[1] = {128}; 

volatile uint32_t msTicks2 = 0;

//moj autorski glupi troche pomysl- 
//zastosowanie struktury, zeby mozna bylo tam wsadzic dzwiek oraz do tego czas przez jaki ma byc odgrywany
struct nutka
{
	int dzwiek;
  	int time;
	//jak bedzie spi to bedzie uint8_t
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
//robi to za nas bibioteka
void initSPI(void) {
	SPIdrv->Initialize(NULL);
	SPIdrv->PowerControl(ARM_POWER_FULL);
	SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_SW | ARM_SPI_DATA_BITS(8),
	10000000);
	SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);   
}
///?????????
// Function to send a byte via SPI, narazie z tym co dzialalo
void sendByteSPI(uint8_t data) {
	
	SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);        /* Transmit some data */  
	SPIdrv->Send(testdata_out, sizeof(testdata_out));
		if(!SPIdrv->GetStatus().busy)
		{	
			//delay(1);
			//SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);    
			SPIdrv->Send(testdata_out, sizeof(testdata_out_0));
			//delay(1);
		}
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
//w tej funkcji bedziemy zapisywac elementy do tablicy jesli flaga zapis bedzie ustalona na 1, innaczej kazdy jeden dzwiek bedzie 
//przesylany do odtwarzania
void jakidzwiek(int xx, int yy)
{
	//kalibracja
	int f[]={262,294,330,349,392,440,493,523};
	int sk=350;
	//int sky=200;//to nie dziala
	short int k;
	xx=xx-500;
	//czy zostal wcisniety klawisz REC
	if(xx<0)
		zapis=0;

	//czy zostal wcisniety klawisz play
	if( xx>8*sk)
	{
		for(k=0;k<zapis;k++)
			zagraj2(melody[k].dzwiek,melody[k].time);
	}
	//ustalenie ktory klawisz zostal nacisniety
	for(k=0;k<8;k++)
	{
		if( xx>k*sk && xx<(k+1)*sk)//y do dodania
		{
			//fun(tab[k]);
			if(zapis>=0 && zapis<100)
			{
				melody[zapis].dzwiek=f[k];
				//narazie tutaj powinien byc czas jak dlugo przycisk byl przycisniety
				melody[zapis].time=1000;
				zapis++;
			}	
			//sprawdzanie czy dziala
			zagraj2(f[k],1000);
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
	jakidzwiek(x[0],x[1]);	
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
void create_ui()
{
	
	int registerStatus=lcdReadReg(OSCIL_ON);
	
	lcdWriteReg(ADRX_RAM,  0);
	lcdWriteReg(ADRY_RAM,  0);
	lcdWriteIndex(DATA_RAM);

	lcdWriteIndex(DATA_RAM);
	rysujprostokat(0,0,320,240,LCDBlue);
	//nie wyglada to ladnie ale przynajmniej malo pamieci zabiera xd
	rysuj('R',0,0);
	rysuj('E',0,8);
	rysuj('C',0,16);
	rysuj('P',300,32);
	rysuj('L',300,40);
	rysuj('A',300,48);
	rysuj('Y',300,56);
	
	short int i;
	for(i=0;i<8;i++)
		rysujprostokat(32+i*32,10,28,130,LCDGreen);
}
void initGPIO()
{
		//konfiguracja GPIO do informowania o przerwaniach-dotykanie ekranu
   PIN_Configure (0,19,0,0,0);
   LPC_GPIOINT->IO0IntEnF=(1<<19);
   NVIC_EnableIRQ(EINT3_IRQn);
   NVIC_SetPriority(EINT3_IRQn,1) ;
   NVIC_GetActive(EINT3_IRQn) ;
}
void init_timer0()
{
  LPC_TIM0->PR=SystemCoreClock/1000000-1;
  LPC_TIM0->MR0=1000-1;
  LPC_TIM0->MCR=3;
  NVIC_EnableIRQ(TIMER0_IRQn);
	//wlaczanie timera
  LPC_TIM0->TCR = 1;
  active = NVIC_GetActive(TIMER0_IRQn);
}
int main()
{
  init_timer0();
  lcdConfiguration();
  init_ILI9325();
  touchpanelInit();
  initDAC();
  initSPI();
  initGPIO();
  create_UI();
  init_GPIO();
	//glowna petla programu
  while(1)
  {}
}
