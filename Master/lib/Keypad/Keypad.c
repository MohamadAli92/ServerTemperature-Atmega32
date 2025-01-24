#include <avr/io.h>

void keypad_init()
{
    // Set column pins as outputs and row pins as inputs
    DDRD |= (1 << DDD7) | (1 << DDD6) | (1 << DDD5);
    DDRD &= ~(1 << DDD3) & ~(1 << DDD2) & ~(1 << DDD1) & ~(1 << DDD0);
    // Enable pull-up resistors for the row pins
    PORTD |= (1 << PORTD3) | (1 << PORTD2) | (1 << PORTD1) | (1 << PORTD0);
}

char keypad_read()
{
    // Define the keypad matrix
    char keypad[4][3] = {
        {'1', '2', '3'},
        {'4', '5', '6'},
        {'7', '8', '9'},
        {'*', '0', '#'}};

    // Array to hold the corresponding row and column values for each key
    int row_pins[4] = {PIND0, PIND1, PIND2, PIND3};
    int column_pins[3] = {PIND5, PIND6, PIND7};

    // Loop through each column and check for key press
    for (int column = 0; column < 3; column++)
    {
        // Activate current column by making it low
        PORTD &= ~(1 << column_pins[column]);
        // Check the row pins for a low signal
        for (int row = 0; row < 4; row++)
            if (!(PIND & (1 << row_pins[row])))
                // Check again to confirm key press
                if (!(PIND & (1 << row_pins[row])))
                    // Return the corresponding keypad character
                    return keypad[row][column];
        // Deactivate current column by making it high
        PORTD |= (1 << column_pins[column]);
    }

    // No key pressed
    return '\0';
}