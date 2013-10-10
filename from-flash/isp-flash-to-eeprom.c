
// install path: /usr/lib/avr/include/avr

#include "isp-flash-to-eeprom.h"

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>

#include "uart_supplemental.h"

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

#define SW PD6
#define SW_D DDRD
#define SW_PIN PIND

#define WE_LOW PORTD &= ~(1<<PD2)
#define WE_HIGH PORTD |= (1<<PD2)
#define OE_DISABLE PORTD |= (1<<PD3)
#define OE_ENABLE PORTD &= ~(1<<PD3)

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

#if 0
  char s [ 30 ];
  sprintf ( s, "B %d\n", PINB );
  logit ( s );
#endif

  if ( PIND & (1<<PD5) ) {
    return (PINB | 128);
  } else {
    return (PINB & ~(128));
  }

}

static inline void set_address_w ( unsigned int w ) {
  // PORTA high address - is only 7 bit
  // PORTC low address - is full 8 bit

  // !!! wtf?!
  // reversed from expected; oh, endian fucked uppedness
  PORTC = ((unsigned char)w);
  PORTA = (unsigned char) ( w >> ((unsigned int)8) );
  PORTA &= ~(128);

#if 0
  char s [ 30 ];
  sprintf ( s, "A %d C %d\n", PORTA, PORTC );
  logit ( s );
#endif
}

int main ( void ) {

  // directions..
  CE_D |= (1<<CE);  // CE
  DDRD |= (1<<PD2); // WE
  DDRD |= (1<<PD3); // RD OE

  // bring eeprom offline
  CE_DISABLE;
  WE_HIGH;
  OE_DISABLE;

  // directions..
  DDRD |= (1<<PD7); // LED
  DDRA = 0xFF;      // address bus high
  DDRC = 0xFF;      // address bus low
  SW_D &= ~(1<<SW); // switch

  // wait for switch
  while ( SW_PIN & (1<<SW) ) {
    // nop
  }

  // serial logging
  serial_setup();

  // big/little endian check
#if 0
  unsigned int biglittle = 0x0A0B;
  char _bl [ 40 ];
  sprintf ( _bl, "AB low %x high %x", ((unsigned char)biglittle), (unsigned char) (biglittle >> ((unsigned int)8)) );
  logit ( _bl );
#endif

#if 0
  // blink test
  while ( 1 ) {

    PORTD &= ~ (1<<PD7);
    _delay_ms ( 200 );

    PORTD |= (1<<PD7);
    _delay_ms ( 200 );

  } // while forever
#endif

  // wait for switch
  while ( SW_PIN & (1<<SW) ) {
    // nop
  }

  unsigned int address;
  char buffer [ 30 ];

  //#define ECHO_MODE 1
#define WRITE_MODE 1
  //#define READ_MODE 1
#define TEST_MODE 1

  // rush: test for echo
#ifdef ECHO_MODE
  char e;

  logit ( "echo begin\n" );
  while ( 1 ) {

#if 0
    e = uart_getchar_block();
    sprintf ( buffer, "echo: %c\n", e );
    logit ( buffer );
#endif
#if 1
    if ( uart_is_getchar_avail() ) {
      logit ( "got something\n" );
      e = uart_getchar_now();
      sprintf ( buffer, "echo: %c\n", e );
      logit ( buffer );
    }
#endif

    _delay_ms ( 10 );

  } // while
#endif

  _data_len = 20;

  // write stuff
#ifdef WRITE_MODE
  logit ( "write mode\n" );

  DDRB = 0xFF;      // data bus
  DDRD |= (1<<PD5); // data bus bit 8
  _delay_ms ( 2 );

  PORTD |= (1<<PD7); // constant LED

  // pulse WE low (then back to high), with OE high (disabled output) -> write
  // address is latched on WE going low, data latched on WE going high

  for ( address = ((unsigned int)0); address < ((unsigned int)_data_len); address++ ) {
    WE_HIGH;
    CE_DISABLE;

    set_address_w ( address );
    set_data_b ( _data [ address ] );
    //set_data_b ( address ); // test increment

    {
      sprintf ( buffer, "a %d w %X ", address, _data [ address ] );
      logit ( buffer );
      sprintf ( buffer, BYTETOBINARYPATTERN, BYTETOBINARY(_data [ address ]) );
      logit ( buffer );

      if ( address % 6 == 0 ) {
        logit ( "\n" );
      } else {
        logit ( "\t" );
      }

    }
    CE_ENABLE;
    WE_LOW;
    _delay_us ( 200 );
    WE_HIGH;
    CE_DISABLE;
    _delay_ms ( 2 );

  } // for
  logit ( "\n" );

  logit ( "write done\n" );

#endif

  // test stuff
#ifdef TEST_MODE
  logit ( "test mode\n" );

  DDRB = 0x00;         // data bus
  DDRD &= ~(1<<PD5);   // data bus bit 8
  PORTB = 0x00;        // clear data pins, just to be safe
  PORTD &= ~(1<<PD5);   // clear data pins, just to be safe
  _delay_ms ( 2 );

  PORTD |= (1<<PD7); // constant LED

  unsigned char good = 1;
  unsigned char b;

  for ( address = ((unsigned int)0); address < ((unsigned int)_data_len); address++ ) {
    //for ( address = 0; address < _data_len; address++ ) {
    set_address_w ( address );
    sprintf ( buffer, "a %d ", address );
    logit ( buffer );
    OE_ENABLE;
    CE_ENABLE;
    _delay_us ( 70 );
    //_delay_ms ( 2 );
    b = get_data_b();
    {
      sprintf ( buffer, "r %X ", b );
      logit ( buffer );
      sprintf ( buffer, BYTETOBINARYPATTERN, BYTETOBINARY(b) );
      logit ( buffer );
      if ( address % 6 == 0 ) {
        logit ( "\n" );
      } else {
        logit ( "\t" );
      }
    }
    if ( b != _data [ address ] ) {
      good = 0;
      //break;
    }
    CE_DISABLE;
    OE_DISABLE;
    _delay_ms ( 10 );
  } // for
  logit ( "\n" );

  if ( ! good ) {
    logit ( "TEST FAIL\n" );
    PORTD &= ~(1<<PD7); // constant LED off
    while(1){}; // spin forever
  }

  logit ( "test OK\n" );

#endif

  // spin
  while ( 1 ) {

    PORTD &= ~ (1<<PD7);
    _delay_ms ( 100 );
    //logit ( "1" );

    PORTD |= (1<<PD7);
    _delay_ms ( 200 );
    //logit ( "2" );

  } // while forever

  return (0);
}

void serial_setup ( void ) {

  // set up the UART to the right baud rate (as defined in BAUD header)
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
#if USE_2X
  UCSR0A |= (1 << U2X0);
#else
  UCSR0A &= ~(1 << U2X0);
#endif

  // date size
  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */ 

  // supported functions
  //UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
  UCSR0B |= ( 1 << TXEN0 );   /* Enable TX */
  UCSR0B |= ( 1 << RXEN0 );   /* Enable TX */

  logit ( "BOOTUP.\n" );

  //
  // map stdio to uart
  //

#if 0
  FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
  stdout /*= stdin */ = &uart_str;
  fprintf(stdout, "Hello world123456789!");
#endif

  return;
}

void logit ( char *foo ) {
  char *p = foo;

  while ( *p != '\0' ) {
    uart_putchar_prewait ( *p );
    p++;
  }

  //uart_putchar_prewait ( '\n' );

  return;
}
