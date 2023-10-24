#include "LPC17xx.h"                    // Device header
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "Driver_USART.h"               // ::CMSIS Driver:USART

/*extern ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART0;
char                   cmd;*/

int main(void)
{
    //conf();
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

    /*USARTdrv->Initialize(NULL);
    USARTdrv->PowerControl(ARM_POWER_FULL);
    USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
    ARM_USART_DATA_BITS_8 |
    ARM_USART_PARITY_NONE |
    ARM_USART_STOP_BITS_1 |
    ARM_USART_FLOW_CONTROL_NONE, 4800);
    USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
    USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
    


    USARTdrv->Send("\nPress Enter to receive a message", 34);
    while(1)

    USARTdrv->Receive(&cmd, 1);
		//kod ascii 13 //jest ok
        if(cmd==13)
        {
            USARTdrv->Send("\nHello World!", 12);
        }
    }*/
    return 0;
}