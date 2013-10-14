
#include "main.h"

#include <stdio.h>
#include <string.h>

#include "lib_uart.h"
#include "serial_server.h"
#include "logging.h"

unsigned char log_format = 1;

void logit ( char *foo ) {
  uart_putstring ( foo );
  return;
}

void logaddress ( unsigned int address, unsigned char value ) {
  char buffer [ 30 ];

  sprintf ( buffer, "%5u: ", address );
  logit ( buffer );

  sprintf ( buffer, "%X ", value );
  logit ( buffer );

#if 0 // binary dump as well?
  sprintf ( buffer, BYTETOBINARYPATTERN, BYTETOBINARY(b) );
  logit ( buffer );
#endif

  if ( log_format ) {

    if ( address != 0 && address % 6 == 0 ) {
      logit ( "\n" );
    } else {
      logit ( "\t" );
    }

  } else {
    logit ( "\n" );
  }

  return;
}

