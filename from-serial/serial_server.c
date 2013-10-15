
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

  // state machine vars .. don't screw with them
  unsigned char char_echo_mode = 1; // default on
  char buffer [ 50 ];               // input from terminal accrued here
  char *args;                       // set on entering non-cmd-parse states; used randomly otherwise
  unsigned char data_buffer [ MAXDATASIZE ];   // as received from terminal
  unsigned int data_len = 0;                   // length of 'data_buffer'

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
          if ( char_echo_mode ) {
            uart_putchar_prewait ( '\n' );
          }
        }

        // parse the buffer; modify to have \0 at first space, so we
        // can find command and assume the rest is args
        args = strchr ( buffer, ' ' );

        if ( args ) {
          *args = '\0';
          args ++;
        } else {
          args = NULL;
        }

        // dispatch command
        if ( strcmp ( buffer, "ohai" ) == 0 ) {
          state = ss_ohai;
        } else if ( strcmp ( buffer, "echo" ) == 0 ) {
          state = ss_echo;
        } else if ( strcmp ( buffer, "receive" ) == 0 ) {
          state = ss_receive;
        } else if ( strcmp ( buffer, "burn" ) == 0 ) {
          state = ss_burn;
        } else if ( strcmp ( buffer, "dump" ) == 0 ) {
          state = ss_dump;
        } else if ( strcmp ( buffer, "help" ) == 0 ) {
          state = ss_help;
        } else if ( strcmp ( buffer, "charecho" ) == 0 ) {
          state = ss_charecho;
        } else if ( strcmp ( buffer, "format" ) == 0 ) {
          state = ss_format;
        } else if ( strcmp ( buffer, "buffer" ) == 0 ) {
          state = ss_buffer;
        } else if ( strcmp ( buffer, "testbuf" ) == 0 ) {
          state = ss_testbuf;
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

      if ( char_echo_mode ) {
        uart_putchar_prewait ( c );
      }

      break;

    case ss_ohai:
      uart_putstring ( "+OHAI\n" );

      log_format = 1;
      char_echo_mode = 1;

      state = ss_ready;
      break;

    case ss_help:

      uart_putstring ( "Enter commands into terminal.\n" );
      uart_putstring ( "\n" );
      uart_putstring ( "ohai -> return OHAI; reset to defaults. (use as first command, always)\n" );
      uart_putstring ( "echo -> enter loop, returning received characters.. forever\n" );
      uart_putstring ( "receive N -> store next N chars (after command return) to buffer\n" );
      uart_putstring ( "burn A -> given a received buffer, burn to address A\n" );
      uart_putstring ( "dump A L -> hexdump from address A of length L\n" );
      uart_putstring ( "charecho -> toggle character echo\n" );
      uart_putstring ( "buffer -> dump the currently received buffer\n" );
      uart_putstring ( "testbuf V -> generate a buffer of 0xV\n" );
      uart_putstring ( "format -> toggle dump formatting; default on. When off, only 1 address/value per line\n" );
      uart_putstring ( "help -> duh\n" );
      uart_putstring ( "\n" );

      state = ss_ready;
      break;

    case ss_charecho:

      if ( char_echo_mode ) {
        char_echo_mode = 0;
      } else {
        char_echo_mode = 1;
      }

      state = ss_ready;
      break;

    case ss_format:

      if ( log_format ) {
        log_format = 0;
      } else {
        log_format = 1;
      }

      state = ss_ready;
      break;

    case ss_echo:
      uart_putstring ( "+ECHO\n" );

      serial_echo_loop_forever();

      state = ss_ready; // should never get here...
      break;

    case ss_testbuf:
      {

        // verify arguments
        if ( ! args ) {
          uart_putstring ( "+BADARGS\n" );
          state = ss_ready;
          break;
        }

        unsigned int b = atoi ( args );

        if ( b > 255 ) {
          uart_putstring ( "+BADARGS\n" );
          state = ss_ready;
          break;
        }

        data_len = 255;

        unsigned int counter;
        for ( counter = 0; counter < data_len; counter++ ) {
          data_buffer [ counter ] = b;
        }

        char log [ 40 ];
        sprintf ( log, "# Generated %d bytes of value %X\n", data_len, b );
        uart_putstring ( log );

      }

      state = ss_ready;
      break;

    case ss_buffer:
      {

        if ( ( data_len == 0 ) ||
             ( data_len > MAXDATASIZE ) )
        {
          uart_putstring ( "+BADARGS\n" );
          state = ss_ready;
          break;
        }

        unsigned int counter;

        for ( counter = ((unsigned int)0); counter < ((unsigned int) data_len); counter++ ) {
          logaddress ( counter, data_buffer [ counter ] );
        } // for
        logit ( "\n" );

        state = ss_ready;
        break;
      }

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
        char log [ 40 ];

        unsigned int counter;
        for ( counter = 0; counter < data_len; counter++ ) {
          data_buffer [ counter ] = uart_getchar_block();
        }

        unsigned int crc = crc32 ( 0, data_buffer, data_len );

        sprintf ( log, "+RECEIVE %X\n", crc );
        uart_putstring ( log );
      }

      state = ss_ready;
      break;

    case ss_burn:
      {
        // burn address
        // TODO: Someday maybe..
        // TODO: burn address [name] [len] (see above)

        // verify arguments
        if ( ! args ) {
          uart_putstring ( "+BADARGS\n" );
          state = ss_ready;
          break;
        }

        unsigned int address = atoi ( args );

        if ( ( address > 65000 ) ||
             ( data_len == 0 ) )
        {
          uart_putstring ( "+BADARGS\n" );
          state = ss_ready;
          break;
        }

        // burn it out
        //eeprom_burn_slow ( address, data_buffer, data_len );
        eeprom_burn_fast ( address, data_buffer, data_len );

        // verify
        if ( eeprom_compare ( address, data_buffer, data_len ) ) {
          uart_putstring ( "+BURNOK\n" );
        } else {
          uart_putstring ( "+BURNFAIL\n" );
        }

      }
      state = ss_ready;
      break;

    case ss_dump:
      // dump address len
      {

        // verify arguments
        if ( ! args ) {
          uart_putstring ( "+BADARGS\n" );
          state = ss_ready;
          break;
        }

        char *split = strchr ( args, ' ' );
        unsigned int address = atoi ( args );
        unsigned int len = atoi ( split + 1 );

        if ( ( address > 65000 ) ||
             ( len == 0 ) )
        {
          uart_putstring ( "+BADARGS\n" );
          state = ss_ready;
          break;
        }

        eeprom_dump ( address, len );

        uart_putstring ( "+OK\n" );

      }

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

