all:	zikburner

ATMEGA=atmega644

CC=avr-gcc
LD=avr-gcc
#CFLAGS=-g -Os -mmcu=${ATMEGA}
CFLAGS=-g -Os -mmcu=${ATMEGA}

# avr-gcc -g -Os -mmcu=atmega644 -c demo.c
# avr-gcc -g -mmcu=atmega8 -o demo.elf demo.o
# avr-objcopy -j .text -j .data -O ihex demo.elf demo.hex
# sudo avrdude -p m644 -c avrispmkII -P usb -U demo.hex

LIBOBJS=lib_uart.o lib_eeprom.o lib_crc32.o
OBJS=main.o serial_server.o logging.o ${LIBOBJS}

clean:
	rm -f zikburner.o
	rm -f zikburner.elf
	rm -f zikburner.hex
	rm -f zikburner.map
	rm -f zikburner
	rm -f *~*
	rm -f ${OBJS}

zikburner:	${OBJS}
	avr-gcc -g -mmcu=${ATMEGA} -o zikburner.elf ${OBJS}
	avr-objcopy -j .text -j .data -O ihex zikburner.elf zikburner.hex

deploy:
	sudo avrdude -p ${ATMEGA} -c avrispmkII -P usb -U zikburner.hex
