#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <SPI.h>

void init_PWM() {

    TCCR0 |= (1 << WGM01) | (1 << WGM00) | (1 << COM01) | (1 << CS01);
    TCCR1A |= (1 << WGM10) | (1 << COM1A1);
    TCCR1B |= (1 << WGM12) | (1 << CS11);
    TCCR2 |= (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (1 << CS21);
    DDRB |= (1 << DDB3);
    DDRD |= (1 << DDD7) | (1 << DDD5) | (1<<DDD4);

    OCR0 = 0;
    OCR1A = 0;
    OCR1B = 0;
    OCR2 = 0;

}

void init_INT() {

    MCUCR |= (0 << ISC01) | (1 << ISC00) | (0 << ISC11) | (1 << ISC10);
    MCUCSR |= (0 << ISC2);

}

uint8_t flag0 = 0;
uint8_t flag1 = 0;
uint8_t flag2 = 0;

void init_ADC() {

    ADMUX = (0 << REFS1) | (1 << REFS0);

    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    ADCSRA |= (1 << ADSC);

}

int dutyCycle_n = 0;
int overall_dc = 0;
int one_dc = 0;
uint8_t prev_cond_count = 0;

uint8_t counter = 0;

int main(int argc, char** argv)
{

    init_PWM();
    init_ADC();
    init_INT();
    init_SPI();

    sei();

    DDRC = 0xFF;
    DDRB |= (1 << DDB1) | (1 << DDB0);
    DDRA |= (1 << DDA4) | (1 << DDA7);


    _delay_ms(500);

    while (1) {

        if (ADCW < 104) dutyCycle_n = 0;
        else if (ADCW >= 104 && ADCW < 206) dutyCycle_n = 1;
        else if (ADCW >= 206 && ADCW < 309) dutyCycle_n = 2;
        else if (ADCW >= 309 && ADCW < 412) dutyCycle_n = 3;
        else if (ADCW >= 412 && ADCW < 514) dutyCycle_n = 4;
        else if (ADCW >= 514 && ADCW < 617) dutyCycle_n = 5;
        else if (ADCW >= 617 && ADCW < 720) dutyCycle_n = 6;
        else if (ADCW >= 720 && ADCW < 822) dutyCycle_n = 7;
        else if (ADCW >= 822 && ADCW < 925) dutyCycle_n = 8;
        else if (ADCW >= 925 && ADCW < 1023) dutyCycle_n = 9;
        else if (ADCW >= 1023) dutyCycle_n = 10;

        overall_dc = 3*(dutyCycle_n * (255/10));


        uint8_t cond_count = 0;
        for (uint8_t i = 0; i < 3; i++)
        {
            if (conds[i]) cond_count++;
        }


        if (cond_count != prev_cond_count) {
            PORTA |= (1 << PORTA7);
            _delay_ms(15);
            SPDR = cond_count;
            PORTA &= ~(1 << PORTA7);
            _delay_ms(100);
            // while (SPDR != 73);
            prev_cond_count = cond_count;
        }

        PORTC = cond_count;

        if (cond_count == 0) one_dc = 257;
        else one_dc = overall_dc / cond_count;
        
        if (one_dc > 255) {
            over_heat = 1;
            OCR0 = 0;
            OCR1A = 0;
            TCCR1A |= (1 << COM1B1);
            OCR2 = 0;
            OCR1B = 120;
        } //error
        else {
            over_heat = 0;
            OCR0 = one_dc;
            OCR1A = one_dc;
            TCCR1A &= ~(1 << COM1B1);
            OCR1B = 0;
            OCR2 = one_dc;
        }


        if (counter == 0) {
            if (flag0 == 1) {
                GICR |= (1 << INT0);
                // _delay_ms(5);
                flag0 = 0;
            } else {
                conds[0] = 0;
                PORTA &= ~(1 << PORTA4);
                flag0 = 1;
            }
        }

        if (counter == 1) {
            if (flag1 == 1) {
                GICR |= (1 << INT1);
                flag1 = 0;
            } else {
                conds[1] = 0;
                PORTB &= ~(1 << PORTB0);
                flag1 = 1;
            }
        }

        if (counter == 2) {
            if (flag2 == 1) {
                GICR |= (1 << INT2);
                flag2 = 0;
            } else {
                conds[2] = 0;
                PORTB &= ~(1 << PORTB1);
                flag2 = 1;
            }
        }

        counter++;
        if (counter == 3) counter = 0;

        _delay_ms(50);

        ADCSRA |= (1 << ADSC);


    }

    return 0;
}

ISR(INT0_vect) {
    flag0 = 1;
    PORTA |= (1 << PORTA4);
    conds[0] = 1;
    GICR &= ~(1 << INT0);
}

ISR(INT1_vect) {
    flag1 = 1;
    PORTB |= (1 << PORTB0);
    conds[1] = 1;
    GICR &= ~(1 << INT1);
}

ISR(INT2_vect) {
    flag2 = 1;
    PORTB |= (1 << PORTB1);
    conds[2] = 1;
    GICR &= ~(1 << INT2);
}


