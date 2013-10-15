
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

#include "lib_eeprom.h"
#include "lib_uart.h"
#include "serial_server.h"
#include "logging.h"

#include "eeprom.h"

void eeprom_reset ( void ) {

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

}

unsigned char eeprom_read ( void ) {
  unsigned char b;

  OE_ENABLE;
  CE_ENABLE;
  //_delay_us ( 70 );
  b = get_data_b();

  CE_DISABLE;
  OE_DISABLE;
  //_delay_us ( 100 );

  return b;
}

unsigned char eeprom_burn_slow_might_not_work_now ( unsigned int address, unsigned char *p, unsigned int len ) {
  unsigned int counter;
  unsigned char value;
  char buffer [ 30 ];

  eeprom_setup_write();

  PORTD |= (1<<PD7); // constant LED

  // pulse WE low (then back to high), with OE high (disabled output) -> write
  // address is latched on WE going low, data latched on WE going high

  counter = 0;
  for ( counter = ((unsigned int)0); counter < ((unsigned int) len); counter++ ) {
    WE_HIGH;
    CE_DISABLE;

    set_address_w ( address );

    //value = address;
    value = p [ counter ];
    set_data_b ( value );

    logaddress ( address, value );

    CE_ENABLE;
    WE_LOW;
    _delay_us ( 100 );
    WE_HIGH;
    CE_DISABLE;
    _delay_us ( 200 );

    // increment address :o
    address++;

  } // for
  logit ( "\n" );

  return ( 1 );
}

// burn_fast() will try a couple strategies..
// i) instead of 'wait'ing for a byte-burn operation to complete, it will poll
// ii) we can look into page burning as well
unsigned char eeprom_burn_fast ( unsigned int address, unsigned char *p, unsigned int len ) {
  unsigned int counter;
  unsigned char value;
  char buffer [ 30 ];

  eeprom_setup_write();

  PORTD |= (1<<PD7); // constant LED

  // pulse WE low (then back to high), with OE high (disabled output) -> write
  // address is latched on WE going low, data latched on WE going high

  counter = 0;
  for ( counter = ((unsigned int)0); counter < ((unsigned int) len); counter++ ) {
    WE_HIGH;
    CE_DISABLE;

    eeprom_setup_write();

    set_address_w ( address );

    //value = address;
    value = p [ counter ];
    set_data_b ( value );

    //logaddress ( address, value );

    CE_ENABLE;
    WE_LOW;

    eeprom_setup_read();

    // spin waiting for IC to confirm the write (by setting value back to expected)
    while ( value != eeprom_read() ) {
      NOP;
    };

    WE_HIGH;
    CE_DISABLE;

    // increment address :o
    address++;

  } // for
  logit ( "\n" );

  return ( 1 );
}

unsigned char eeprom_compare ( unsigned int address, unsigned char *p, unsigned int len ) {
  unsigned int counter;
  char buffer [ 30 ];

  eeprom_setup_read();

  PORTD |= (1<<PD7); // constant LED

  unsigned char good = 1;
  unsigned char b;

  for ( counter = ((unsigned int)0); counter < ((unsigned int) len); counter++ ) {
    set_address_w ( address );

    OE_ENABLE;
    CE_ENABLE;
    _delay_us ( 70 );
    b = get_data_b();

    if ( b != p [ counter ] ) {
      good = 0;
      break;
    }
    CE_DISABLE;
    OE_DISABLE;
    _delay_us ( 100 );

    // increment address :o
    address++;

  } // for

  if ( ! good ) {
    return ( 0 );
  }

  return ( 1 );
}

void eeprom_dump ( unsigned int address, unsigned int len ) {
  unsigned int counter;

  eeprom_setup_read();

  PORTD |= (1<<PD7); // constant LED

  unsigned char good = 1;
  unsigned char b;

  for ( counter = ((unsigned int)0); counter < ((unsigned int) len); counter++ ) {

    set_address_w ( address );
    b = eeprom_read();

    logaddress ( address, b );

    // increment address :o
    address++;

  } // for
  logit ( "\n" );

  return;
}
