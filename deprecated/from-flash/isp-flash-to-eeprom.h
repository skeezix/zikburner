
#ifndef h_isp_flash_to_eeprom_h
#define h_isp_flash_to_eeprom_h

//#define F_CPU 1000000UL  /* 1 MHz CPU clock */
#define F_CPU 8000000UL  /* 8 MHz CPU clock */
//#define F_CPU 20000000UL  /* 20 MHz CPU clock */

#define BAUD 9600

void serial_setup ( void );


#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
    (byte & 0x40 ? 1 : 0), \
    (byte & 0x20 ? 1 : 0), \
    (byte & 0x10 ? 1 : 0), \
    (byte & 0x08 ? 1 : 0), \
    (byte & 0x04 ? 1 : 0), \
    (byte & 0x02 ? 1 : 0), \
    (byte & 0x01 ? 1 : 0) 
void logit ( char *foo );

#endif
