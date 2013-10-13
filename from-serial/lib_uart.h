#ifndef h_uart_h
#define h_uart_h

void uart_putchar_prewait ( char c );
void uart_putchar_postwait ( char c );
char uart_getchar_now ( void ); // return whatever is in uart register now
char uart_getchar_block ( void );
unsigned char uart_is_getchar_avail ( void ); // check if a char is waiting

void uart_putstring ( char *s );

#endif
