
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// install path: /usr/lib/avr/include/avr

#define F_CPU 1000000UL  /* 1 MHz CPU clock */
//#define F_CPU 8000000UL  /* 8 MHz CPU clock */
//#define F_CPU 20000000UL  /* 20 MHz CPU clock */

#include <util/delay.h>
#include <avr/io.h>

// direction
//   ADR (analog direction register) and DDR (digital direction register)
//   DDRx -> set value to 1, sets dir to write; low is read
//   PORTxn -> set high; if write, sets to high; if read, sets pull up resistor
// interupts
//   GIMSK = _BV (INT0); // int - Enable external interrupts int0
//   MCUCR = _BV (ISC01); // int - INT0 is falling edge
//   sei(); // int - Global enable interrupts

#define CE PD4
#define CE_D DDRD
#define CE_PORT PORTD
#define CE_DISABLE CE_PORT |= (1<<CE)
#define CE_ENABLE CE_PORT &= ~(1<<CE)
#define WE_DISABLE PORTD &= ~(1<<PD2)
#define WE_ENABLE PORTD |= (1<<PD2)

extern unsigned int _data_len;
extern unsigned char _data[];

static inline void set_data_b ( unsigned char b ) {
  PORTB = b;
  if (b & 128) {
    PORTD |= (1<<PD5);
  } else {
    PORTD &= ~(1<<PD5);
  }
  return;
}

int main ( void ) {

  // directions..
  CE_D |= (1<<CE);  // CE
  DDRD |= (1<<PD2); // WE
  DDRD |= (1<<PD3); // RD
  DDRD |= (1<<PD7); // LED
  DDRB = 0xFF;      // data bus
  DDRD |= (1<<PD5); // data bus bit 8
  DDRA = 0xFF;      // address bus high
  DDRC = 0xFF;      // address bus low

  // bring eeprom offline
  CE_DISABLE;
  WE_DISABLE;


  while ( 1 ) {

    PORTD &= ~ (1<<PD7);
    _delay_ms ( 100 );

    PORTD |= (1<<PD7);
    _delay_ms ( 200 );

  } // while forever

  return (0);
}
