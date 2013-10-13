
#ifndef h_eeprom_h
#define h_eeprom_h

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
    return (PINB | 128);
  } else {
    return (PINB & ~(128));
  }

}

static inline void set_address_w ( unsigned int w ) {
  // PORTA high address - is only 7 bit
  // PORTC low address - is full 8 bit

  PORTC = ((unsigned char)w);
  PORTA = (unsigned char) ( w >> ((unsigned int)8) );
  PORTA &= ~(128);

}

#endif
