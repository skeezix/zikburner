
#include "main.h"

#include "lib_uart.h"
#include "serial_server.h"
#include "logging.h"

void logit ( char *foo ) {
  uart_putstring ( foo );
  return;
}
