#include <SPI.c>

void SPI_init();
void SPI_transmit(int data);
ISR(SPI_STC_vect);
