/* Phantom board parameters. */

#include "board.h"

/* Specifies the ports and pin numbers for the rows */
uint8_t *const  row_ddr[NROW] = { _DDRB,  _DDRB,  _DDRB,  _DDRB,  _DDRB,  _DDRB};
uint8_t *const row_port[NROW] = {_PORTB, _PORTB, _PORTB, _PORTB, _PORTB, _PORTB};
uint8_t *const  row_pin[NROW] = { _PINB,  _PINB,  _PINB,  _PINB,  _PINB,  _PINB};
const uint8_t   row_bit[NROW] = { _BIT0,  _BIT1,  _BIT2,  _BIT3,  _BIT4,  _BIT5};

/* Specifies the ports and pin numbers for the columns */
uint8_t *const  col_ddr[NCOL] = { _DDRD,  _DDRC,  _DDRC,  _DDRD,  _DDRD,  _DDRE,
				  _DDRF,  _DDRF,  _DDRF,  _DDRF,  _DDRF,  _DDRF,
				  _DDRD,  _DDRD,  _DDRD,  _DDRD,  _DDRD};
uint8_t *const col_port[NCOL] = {_PORTD, _PORTC, _PORTC, _PORTD, _PORTD, _PORTE,
				 _PORTF, _PORTF, _PORTF, _PORTF, _PORTF, _PORTF,
				 _PORTD, _PORTD, _PORTD, _PORTD, _PORTD};
const uint8_t   col_bit[NCOL] = { _BIT5,  _BIT7,  _BIT6,  _BIT4,  _BIT0,  _BIT6,
				  _BIT0,  _BIT1,  _BIT4,  _BIT5,  _BIT6,  _BIT7,
				  _BIT7,  _BIT6,  _BIT1,  _BIT2,  _BIT3};

inline void pull_column(int col) {
  *col_port[col] &= ~col_bit[col];
}

inline void release_column(int col) {
  *col_port[col] |=  col_bit[col];
}

inline void update_leds(void) {
  PORTB = (PORTB & 0x3F) | ((keyboard_leds << 5) & 0xC0);
  DDRB  = (DDRB  & 0x3F) | ((keyboard_leds << 5) & 0xC0);
}

/* PORTB is set as input with pull-up resistors
   PORTC,D,E,F are set to high output */
void setup_io_pins(void) {
  uint8_t row, col;
  for(row = 0; row < NROW; row++) {
    *row_ddr[row]  &= ~row_bit[row];
    *row_port[row] |=  row_bit[row];
  }
  for(col = 0; col < NCOL; col++) {
    *col_ddr[col]  |= col_bit[col];
    *col_port[col] |= col_bit[col];
  }
}

/* LEDs are on output compare pins OC1B OC1C
   This activates fast PWM mode on them.
   Prescaler 256 and 8-bit counter results in
   16000000/256/256 = 244 Hz blink frequency.
   LED_A: Caps Lock
   LED_B: Scroll Lock  */
/* Output on PWM pins are turned off when the timer 
   reaches the value in the output compare register,
   and are turned on when it reaches TOP (=256). */
void setup_leds(void) {
  TCCR1A |=      // Timer control register 1A
    (1<<WGM10) | // Fast PWM 8-bit
    (1<<COM1B1)| // Clear OC1B on match, set at TOP
    (1<<COM1C1); // Clear OC1C on match, set at TOP
  TCCR1B |=      // Timer control register 1B
    (1<<WGM12) | // Fast PWM 8-bit
    (1<<CS12);   // Prescaler 256
  OCR1B = 250;    // Output compare register 1B
  OCR1C = 250;    // Output compare register 1C
  // LEDs: LED_A -> PORTB6, LED_B -> PORTB7
  DDRB  &= 0x3F;
  PORTB &= 0x3F;
}

/* Timer0 is used to call the debounce algorithm
   at regular intervals via an interrupt.
   The interrupt is triggered when the timer reaches 
   the value in the output compare register. In CTC
   mode this also resets the timer. */
void setup_bounce_timer(void) {
  TCCR0A |=      // Timer control register 0A
    (1<<WGM01);  // Set CTC, clear timer on compare
  TCCR0B |=      // Timer control register 0A
    //  (1<<CS00) |  // Prescaler 1024, frequency 15.6kHz
    (1<<CS02);   // Prescaler 256, frequency 62.5kHz
  TIMSK0 |=      // Timer interrupt mask register 0
    (1<<OCIE0A); // Enable timer interrupt on compare match with OCR0A
  OCR0A = 32;    // Output compare register 0A
}

/* Simple LED blinker for debugging use */
void toggle_leds(void) {DDRB  = PORTB = ~DDRB & 0b11000000;}
