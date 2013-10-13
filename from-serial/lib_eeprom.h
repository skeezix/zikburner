
#ifndef h_lib_eeprom_h
#define h_lib_eeprom_h

// reset() -- take eeprom to safe state; no reading/writing going on, directions are set up, etc.
void eeprom_reset ( void );

// burn_slow() -- attempt to burn a buffer to the target device; it is 'slow' in the sense
// where no attempt is made to burn rapidly using 'page write' or the like; it takes its
// time.
//
//   address -- target
//   p -- buffer
//   len -- len to iterate across buffer
//
// Returns >0 on success, 0 on error
unsigned char eeprom_burn_slow ( unsigned int address, unsigned char *p, unsigned int len );

// compare() -- given a buffer and len, read starting at the given address and ensure
// target device matches the buffer.
//
// Returns >0 on success (they match), 0 on error (something wasn't a match)
unsigned char eeprom_compare ( unsigned int address, unsigned char *p, unsigned int len );

// dump() -- display hexdump to serial
void eeprom_dump ( unsigned int address, unsigned int len );

#endif
