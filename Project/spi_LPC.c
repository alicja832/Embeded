#include "LPC17xx.h"
// Function to initialize SPI for communication with DAC
void initSPI(void) {
	//chyba trzeba jeszcze skonfigurowac piny
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

// Function to initialize DAC
void initDAC(void) {
    // Enable DAC output
	
   LPC_DAC->DACCTRL |= (1 << 2);
}

// Function to send data to DAC
void sendToDAC(unsigned short data) {
    LPC_DAC->DACR = (data << 6) & 0xFFC0;
}

int main(void) {
    // Assuming you have audio data prepared as an array
    unsigned short audioData[] = { /* Your audio data goes here */ };
    int dataSize = sizeof(audioData) / sizeof(audioData[0]);

    // Initialize SPI and DAC
    initSPI();
    initDAC();

    // Send audio data to DAC
    for (int i = 0; i < dataSize; ++i) {
        sendToDAC(audioData[i]);
        // Add a suitable delay if needed between data points
    }

    while (1) {
        // Your main program loop
    }

    return 0;
}

