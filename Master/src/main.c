#include <avr/io.h>
#include <util/delay.h>
#include <LCD.h>
#include <Keypad.h>
#include <SPI.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int strCmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void write_lcd(char str[]) {
  init_LCD();

  // LCD_cmd(0x0F);

  unsigned char i = 0;
  
  unsigned char c = str[i];

  while (c != '\0')
  {
    LCD_write(c, 1);
    i++;
    c = str[i];
  }

}

void init_INT() {
  MCUCSR |= (0 << ISC2);
  GICR |= (1 << INT2);
}

int main(void) {

  init_INT();

  DDRB |= (1 << DDB1);
  DDRB |= (1 << DDB0);
  DDRB |= (1 << DDB3);

  unsigned char i = 0;
  unsigned char locked_string[30] = {"Password: < 12 char \0"};


  DDRA = 0xFF;
  DDRC = 0x07;
  init_LCD();

  // LCD_cmd(0x0F);

  keypad_init();


  SPI_init();

  sei();
  
  LCD_cmd(0x01);

  unsigned char c = locked_string[i];

  while (c != '\0')
  {
    LCD_write(c, 1);
    i++;
    c = locked_string[i];
  }


  uint8_t counter = 0;
  char input_code[13];
  uint8_t locked = 1;
  uint8_t remain_atempts = 3;
  char from_back = 0;
  char page = 1;
  char page_str[2] = "1";

  while (1)
  {
    if (locked) {
      char key = keypad_read();

      if (key != '#' && key != '*' && key != 0) {
        LCD_write(key, 0);
        input_code[counter++] = key;
      } else if (key == '*') {
          if (counter > 0) {
            LCD_delete();
            counter--;
          }
      } else if (key == '#') {
        input_code[counter++] = '\0';

        
        uint8_t c = 0;
        send_pass(1);
        while (input_code[c] != '\0')
        {
          send_pass(input_code[c]);
          c++;
        }
        send_pass(0);
        _delay_ms(5);
        get_valid();

        while (!correct_password) _delay_ms(1);
        if (correct_password == 2) {
          write_lcd("Successful :)\0");
          locked = 0;
          _delay_ms(200);
        }

        else {
          correct_password = 0;
          if (remain_atempts == 3) write_lcd("Wrong Password, 2 more attempts.\0");
          else if (remain_atempts == 2) write_lcd("Wrong Password, 1 more attempts.\0");
          else if (remain_atempts == 1) {
            counter = 0;
            write_lcd("Locked for 30 seconds :(\0");
            remain_atempts = 3;
            _delay_ms(3000);
            write_lcd("Password: < 12 char \0"); 
            continue;
          } 
          remain_atempts--;
          _delay_ms(200);
          counter = 0;
          write_lcd("Password: < 12 char \0"); 
        };

      }
    }
    else {
      char key;
      if (!from_back){
        write_lcd("1-Motor Status      2-Temperature");
        key = keypad_read();
      } else key = '1';
      while (1)
      {
        if (key == '1' || key == '2')
        {
          break;
        } else key = keypad_read();
      }
      if (key == '1') {
        char str[30] = {"Motor \0"};
        itoa(page, page_str, 10);
        strcat(str, page_str);
        strcat(str, ": ");
        // char str[30] = {"Motor 1: \0"};
        // USART
        ready_status(page);
        _delay_ms(10);
        get_status();
        if (status == 28) strcat(str, "Overheat");
        else if (!status) strcat(str, "FAIL");
        else strcat(str, "OK");
        if (status != 28) strcat(str, " | ");
        // USART
        ready_dc_m(page);
        _delay_ms(10);
        get_dc_m(40);
        int dc_num = round((recievec_dc*100)/255);
        if (recievec_dc <= 25.5 || !status) dc_num = 0;
        char dc_str[5];
        itoa(dc_num, dc_str, 10);

        if (status != 28) {
          strcat(str, dc_str);
          strcat(str, "%");
        }
        if (!status) strcat(str, "  #: Next | 0: Exit");
        else strcat(str, "   #: Next | 0: Exit");
        write_lcd(str);

        while (1)
        {
          if (key == '#')
          {
            if (page == 3) page = 1;
            else page++;
            from_back = 1;
            break;
          }
          else if (key == '0') {
            from_back = 0;
            page = 1;
            page_str[0] = '1';
            break;
          }
          else key = keypad_read();
        }

      } else {
        // SPI
        ready_temp_low();
        _delay_ms(10);
        get_temp();
        uint16_t full_temp = temperature;
        ready_temp_hi();
        _delay_ms(10);
        get_temp();
        full_temp |= (temperature << 8);
        int temp_num = full_temp/10;
        if (temp_num >= 72) temp_num -= 2;
        else if (temp_num >= 33) temp_num--;
        char temp_str[5];
        char str[30] = {"Temperature: \0"};
        itoa(temp_num, temp_str, 10);
        strcat(str, temp_str);
        strcat(str, "     0: Exit");
        write_lcd(str);
        while (1) {
            if (key == '0') {
            from_back = 0;
            page = 1;
            page_str[0] = '1';
            break;
          }
          else key = keypad_read();

        }
      }
    }
  }
    
}

ISR(INT2_vect) {

  get_cond();
  if (cond == 3) {
    PORTB |= (1 << PORTB0);
    PORTB &= ~(1 << PORTB1);
    PORTB &= ~(1 << PORTB3);
  } else if (cond == 2) {
    PORTB &= ~(1 << PORTB0);
    PORTB |= (1 << PORTB1);
    PORTB &= ~(1 << PORTB3);
  } else if (cond == 1) {
    PORTB &= ~(1 << PORTB0);
    PORTB |= (1 << PORTB1);
    PORTB &= ~(1 << PORTB3);
  } else {
    PORTB &= ~(1 << PORTB0);
    PORTB &= ~(1 << PORTB1);
    PORTB |= (1 << PORTB3);
  }
}
