#include <avr/io.h>
#include <util/delay.h>

#define LCD_DATA PORTA
#define ctrl PORTC
// #define en PIND.2                        
// #define rw PIND.1                     
// #define rs PIND.0

void LCD_cmd(unsigned char cmd);
void init_LCD(void);
void LCD_write(unsigned char datam, unsigned char fast);
void init_LCD(void)
{
    LCD_cmd(0x38);         // 8-bit mode                   
    _delay_ms(1);
    LCD_cmd(0x01);         // clear the screen                     
    _delay_ms(1);
    LCD_cmd(0x0E);         // turn on the cursor                
    _delay_ms(1);
    LCD_cmd(0x80);         // move cursor to the first place of the first row   
    _delay_ms(1);
    return;
}

void LCD_cmd(unsigned char cmd)
{
    LCD_DATA=cmd;
    ctrl=0x04;              // Register Select = 0, Read/Write = 0, Enable = 1
    _delay_ms(1);
    ctrl=0x00;              // Enable = 0
    _delay_ms(50);
    return;
}

void LCD_write(unsigned char data, unsigned char fast)
{
    LCD_DATA= data;
    ctrl=0x05;               // Register Select = 1, Read/Write = 0, Enable = 1
    _delay_ms(1);
    ctrl=0x01;                 // Enable = 0
    if (!fast) _delay_ms(50);
    else _delay_ms(1);
    return ;
}

void LCD_delete() {
    LCD_cmd(0x10);  // Move the cursor to the left
    LCD_write(' ', 1); // Write a space
    LCD_cmd(0x10);  // Move the cursor to the left
}