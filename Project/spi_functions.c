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
///?????????
// Function to send a byte via SPI
void sendByteSPI(unsigned short data) {
    LPC_SPI->SPDR = data;
	//tutaj nie za bardzo wiem o co chodzi
    while (!(LPC_SPI->SPDR & (1 << 7))); // Wait for SPIF flag
}
