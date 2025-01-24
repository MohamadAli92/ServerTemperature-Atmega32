#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
// #include <LCD.h>

uint8_t send = 1;
uint8_t correct_password = 0;
uint8_t check_val = 0;
uint16_t temperature = 32;
uint8_t recievec_dc = 30;
uint8_t status = 0;
uint8_t cond = 3;

// Initialize SPI Master Device (with SPI interrupt)
void SPI_init()
{

    // Set MOSI, SCK, SS as Output (PB5, PB7, PB4) and MISO as Input (PB6)
    DDRB |= (1 << DDB7) | (0 << DDB6) | (1 << DDB5) | (1 << DDB4);
    // Activate Pull-Up on MISO (PB6) to set it to high level (1)
    // PORTB |= (1 << PORTB4);

    PORTB &= ~(1 << PORTB4);

    // Enable SPI, Enable SPI Interrupt, Set as Master Device, Set SCK Frequency to Fosc/128 (125kHz)
    SPCR |= (1 << SPIE) | (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);


    // Enable Global Interrupts
    sei();
}

void send_pass(int data){
    // Start Transmission
    _delay_ms(1);
    SPDR = data;
}

void get_valid(){
    SPDR = 125;
}

void ready_temp_low() {
    SPDR = 176;
}

void ready_temp_hi() {
    SPDR = 177;
}

void get_temp() {
    SPDR = 125;
    _delay_ms(10);
    temperature = SPDR;
}

void ready_dc_m(int m_number) {
    if (m_number == 1) SPDR = 10;
    if (m_number == 2) SPDR = 20;
    if (m_number == 3) SPDR = 30;
}

void get_dc_m() {
    SPDR = 40;
    _delay_ms(10);
    recievec_dc = SPDR;
}

void ready_status(int m_number) {
    if (m_number == 1) SPDR = 41;
    if (m_number == 2) SPDR = 42;
    if (m_number == 3) SPDR = 43;
}

void get_status() {
    SPDR = 60;
    _delay_ms(10);
    status = SPDR;
}

void get_cond() {
    SPDR = 73;
    _delay_ms(10);
    cond = SPDR;
}


// SPI Interrupt Service Routine
int counter = 0;
ISR(SPI_STC_vect)
{
    counter++;
    // if (send) {

        PORTB |= (1 << PORTB4); // Deselect Slave
        while (((SPSR >> SPIF) & 1) == 0);
        PORTB &= ~(1 << PORTB4); // Select Slave
        // check_val = ~check_val;
        // if (check_val){


        uint8_t received_data = SPDR;
        if (received_data == 2) {
            // PORTB |= (1 << PORTB1);
            correct_password = 2;
        } else if (received_data == 1) correct_password = 1;
        // }
        // send = 0;
    // } else {
    //     PORTB &= ~(1 << PORTB4); // Deselect Slave
    //     if (SPDR == 2) correct_password = 2;
    //     else correct_password = 1;
    //     send = 1;
    // }
}