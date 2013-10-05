
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

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

#define SW PD6
#define SW_D DDRD
#define SW_PIN PIND

#define CE_ENABLE CE_PORT &= ~(1<<CE)
#define WE_LOW PORTD &= ~(1<<PD2)
#define WE_HIGH PORTD |= (1<<PD2)
#define OE_ENABLE PORTD |= (1<<PD3)
#define OE_DISABLE PORTD &= ~(1<<PD3)

extern unsigned int _data_len;
extern unsigned char _data[];

static inline void set_data_b ( unsigned char b ) {

  PORTB = b;
  if (b & 128) {
    PORTD |= (1<<PD5);
  } else {
    PORTD &= ~(1<<PD5);
  }

}

static inline unsigned char get_data_b ( void ) {

  if ( PIND & (1<<PD5) ) {
    return (PINB & ~(128)) | 128;
  } else {
    return (PINB & ~(128));
  }

}

static inline void set_address_w ( unsigned int w ) {
  PORTC = w & 0xFF;
  PORTA = w >> 8;
}

int main ( void ) {

  // directions..
  CE_D |= (1<<CE);  // CE
  DDRD |= (1<<PD2); // WE
  DDRD |= (1<<PD3); // RD OE
  DDRD |= (1<<PD7); // LED
  DDRB = 0xFF;      // data bus
  DDRD |= (1<<PD5); // data bus bit 8
  DDRA = 0xFF;      // address bus high
  DDRC = 0xFF;      // address bus low
  SW_D &= ~(1<<SW); // switch

  // bring eeprom offline
  CE_DISABLE;
  WE_HIGH;

  // wait for switch
  while ( SW_PIN & (1<<SW) ) {
    // nop
  }

  unsigned int address;

//#define WRITE_MODE 1
//#define READ_MODE 1
#define TEST_MODE 1

  // write stuff
#ifdef WRITE_MODE
  OE_DISABLE;
  _delay_ms ( 2 );

  PORTD |= (1<<PD7); // constant LED

  // pulse WE low (then back to high), with OE high (disabled output) -> write
  // address is latched on WE going low, data latched on WE going high

  for ( address = 0; address < 100; address++ ) {
    set_address_w ( address );
    CE_ENABLE;
    WE_LOW;
    set_data_b ( _data [ address ] );
    WE_HIGH;
    CE_DISABLE;
    _delay_ms ( 20 );
  } // for

#endif

  // test stuff
#ifdef TEST_MODE
  CE_ENABLE;
  WE_HIGH;

  PORTD |= (1<<PD7); // constant LED

  unsigned char good = 1;

  for ( address = 0; address < 100; address++ ) {
    set_address_w ( address );
    OE_ENABLE;
    _delay_ms ( 2 );
    if ( get_data_b() != _data [ address ] ) {
      good = 0;
      break;
    }
    OE_DISABLE;
  } // for

  if ( ! good ) {
    PORTD &= ~(1<<PD7); // constant LED off
    while(1){}; // spin forever
  }

#endif

  // spin
  while ( 1 ) {

    PORTD &= ~ (1<<PD7);
    _delay_ms ( 100 );

    PORTD |= (1<<PD7);
    _delay_ms ( 200 );

  } // while forever

  return (0);
}
