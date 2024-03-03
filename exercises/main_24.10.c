#include "LPC17xx.h"                    // Device header
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "Driver_USART.h"               // ::CMSIS Driver:USART


int main(void)
{
    PIN_Configure(0,2,1,0,0);
    PIN_Configure(0,3,1,0,0);

    LPC_UART0->LCR=3|(1<<7);
    LPC_UART0->DLL=27;
    LPC_UART0->DLM=0;
    LPC_UART0->LCR=3;

    LPC_UART0->LCR=3;

    char *x="Ala ma kota";
    while(*x!='\0')
    {
        LPC_UART0->THR=*x;
        x++;
    }


    while(1)
    {
        if(LPC_UART0->LSR&1)
            LPC_UART0 -> THR = LPC_UART0 -> RBR;
    }

   
    return 0;
}