// #include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

// #include "password.h"

int locked = 1;
int slave_sent = 0;
char conds[3] = {1, 1, 1};
uint8_t over_heat = 0;

int strCmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void init_SPI()
{
    // Set MISO as output, all others as input (MOSI, SCK, SS)
    DDRB |= (0 << DDB7) | (1 << DDB6) | (0 << DDB5) | (0 << DDB4) | (1 << DDB3);
    // Enable SPI, Enable Interrupt, Set as Slave, Set Clock Rate to fck/128
    SPCR |= (1 << SPIE) | (1 << SPE) | (0 << MSTR) | (1 << SPR1) | (1 << SPR0);
    // Enable Global Interrupts (I-bit) in SREG (Status Register)
}

// SPI Interrupt Service Routine
char input_code[13];
uint8_t c;

ISR(SPI_STC_vect)
{
    
    if (locked) {
        char pass_code = 12;
        uint8_t received_data = SPDR; // Read the received data
        

        if (received_data == 1) {
            c = 0;
            // PORTB |= (1 << PORTB1);

        }
        else if (received_data != '\0')
        {
            input_code[c] = received_data;
            c++;
        } else if (received_data == 0) {
            input_code[c] = '\0';
            if (atoi(input_code) == pass_code) {
                locked = 0;
                SPDR = 2;
            } else SPDR = 1;
        }
    } else {
        if (SPDR == 176) {
            SPDR = ADCL;
        } else if (SPDR == 177) {
            SPDR = ADCW >> 8;
        }
        else if (SPDR == 10) SPDR = over_heat ? 28 : OCR0;
        else if (SPDR == 20) SPDR = over_heat ? 28 : OCR1A;
        else if (SPDR == 30) SPDR = over_heat ? 28 : OCR2;
        else if (SPDR == 41) SPDR = over_heat ? 28 : conds[0];
        else if (SPDR == 42) SPDR = over_heat ? 28 : conds[1];
        else if (SPDR == 43) SPDR = over_heat ? 28 : conds[2];
    }
    
}