
#ifndef h_isp_flash_to_eeprom_h
#define h_isp_flash_to_eeprom_h

//#define F_CPU 1000000UL  /* 1 MHz CPU clock */
#define F_CPU 8000000UL  /* 8 MHz CPU clock */
//#define F_CPU 20000000UL  /* 20 MHz CPU clock */

#define BAUD 9600



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


void blinkit ( void );


#endif
