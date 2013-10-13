
#include "main.h"

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <string.h>

#include "lib_uart.h"
#include "serial_server.h"
#include "logging.h"
#include "lib_crc32.h"

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

  logit ( "+BOOTUP\n" );

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

void serial_loop_forever ( void ) {
  serial_state_e state;
  char c;

  char buffer [ 50 ];
  char *args;
  unsigned char data_buffer [ MAXDATASIZE ];
  unsigned int data_len = 0;
  char log [ 80 ];
  unsigned int address;

  // test: echo time..
  //serial_echo_loop_forever();

  state = ss_ready;

  while ( 1 ) {

    switch ( state ) {

    case ss_ready:
      uart_putstring ( "+READY\n" );
      // reset for next go..
      buffer [ 0 ] = '\0';
      state = ss_cmd_build;
      break;

    case ss_cmd_build:

      c = uart_getchar_block();

      if ( c == '\n'  || c == '\r') {

        if ( c == '\r' ) {
          uart_putchar_prewait ( '\n' );
        }

        // parse the command out of the buffer
        char command [ 10 ];
        char *space = strchr ( buffer, ' ' );

        if ( space ) {
          strncpy ( command, buffer, space - buffer );
          args = space + 1;
        } else {
          // yay, we're unsafe.. but its a micro we're programming, using a client..
          // we'll get over it.
          strcpy ( command, buffer );
          args = NULL;
        }

        // dispatch command
        if ( strcmp ( command, "ohai" ) == 0 ) {
          state = ss_ohai;
        } else if ( strcmp ( command, "echo" ) == 0 ) {
          state = ss_echo;
        } else if ( strcmp ( command, "receive" ) == 0 ) {
          state = ss_receive;
        } else if ( strcmp ( command, "burn" ) == 0 ) {
          state = ss_burn;
        } else if ( strcmp ( command, "dump" ) == 0 ) {
          state = ss_dump;
        } else {
          uart_putstring ( "+BADCOMMAND\n" );
          buffer [ 0 ] = '\0';
        }

      } else {

        if ( strlen ( buffer ) < 49 ) {
          sprintf ( strchr ( buffer, '\0' ), "%c", c );
        } else {
          // uuuh, overrun, .. reset?!
          uart_putstring ( "+GARBAGE\n" );
          buffer [ 0 ] = '\0';
        }

      }

      uart_putchar_prewait ( c );

      break;

    case ss_ohai:
      uart_putstring ( "+OHAI\n" );
      state = ss_ready;
      break;

    case ss_echo:
      uart_putstring ( "+ECHO\n" );
      serial_echo_loop_forever();
      break;

    case ss_receive:
      // receive LEN
      // TODO: someday, possibly..
      // TODO: receive LEN [NAME] --> allow naming the incoming buffer for re-use?

      // verify arguments
      if ( ! args ) {
        uart_putstring ( "+BADARGS\n" );
        state = ss_ready;
        break;
      }

      data_len = atoi ( args );

      if ( ( data_len == 0 ) ||
           ( data_len > MAXDATASIZE ) )
      {
        uart_putstring ( "+BADARGS\n" );
        state = ss_ready;
        break;
      }

      //sprintf ( log, "# Receive %d bytes\n", data_len );
      //uart_putstring ( log );

      {
        unsigned int counter;
        unsigned char c;
        for ( counter = 0; counter < data_len; counter++ ) {
          c = uart_getchar_block();
          data_buffer [ counter ] = c;
        }
      }

      {
        unsigned int crc = crc32 ( 0, data_buffer, data_len );

        sprintf ( log, "+RECEIVE %X\n", crc );
        uart_putstring ( log );
      }

      state = ss_ready;

      break;

    case ss_burn:
      // burn address
      // TODO: Someday maybe..
      // TODO: burn address [name] [len] (see above)

      // verify arguments
      if ( ! args ) {
        uart_putstring ( "+BADARGS\n" );
        state = ss_ready;
        break;
      }

      address = atoi ( args );

      if ( ( address > 65000 ) ||
           ( data_len == 0 ) )
      {
        uart_putstring ( "+BADARGS\n" );
        state = ss_ready;
        break;
      }

      sprintf ( log, "# Burn %d bytes to address %d\n", data_len, address );
      uart_putstring ( log );

      // burn it out
      eeprom_burn_slow ( address, data_buffer, data_len );

      // verify
      uart_putstring ( "# Verifying..\n" );
      if ( eeprom_compare ( address, data_buffer, data_len ) ) {
        uart_putstring ( "+BURNOK\n" );
      } else {
        uart_putstring ( "+BURNFAIL\n" );
      }

      state = ss_ready;

      break;

    case ss_dump:
      // dump address len

      // verify arguments
      if ( ! args ) {
        uart_putstring ( "+BADARGS\n" );
        state = ss_ready;
        break;
      }

      char *split = strchr ( args, ' ' );
      address = atoi ( args );
      unsigned int len = atoi ( split + 1 );

      if ( ( address > 65000 ) ||
           ( len == 0 ) ||
           ( len > 256 ) )
      {
        uart_putstring ( "+BADARGS\n" );
        state = ss_ready;
        break;
      }

      sprintf ( log, "# Dump %d bytes starting at %d\n", len, address );
      uart_putstring ( log );

      eeprom_dump ( address, len );

      uart_putstring ( "+OK\n" );

      state = ss_ready;

      break;

    } // switch



  } // while

  return;
}

// rush: test for echo
void serial_echo_loop_forever ( void ) {
  char buffer [ 30 ];
  char e;

  while ( 1 ) {

#if 0
    e = uart_getchar_block();
    sprintf ( buffer, "echo: %c\n", e );
    logit ( buffer );
#endif
#if 1
    if ( uart_is_getchar_avail() ) {
      e = uart_getchar_now();
      sprintf ( buffer, "echo: %c\n", e );
      logit ( buffer );
    }

    _delay_ms ( 1 );
#endif

  } // while

}

