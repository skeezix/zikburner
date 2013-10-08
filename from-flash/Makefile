all:	isp-flash-to-eeprom

#ATMEGA=atmega128
ATMEGA=atmega644

CC=avr-gcc
LD=avr-gcc
#CFLAGS=-g -Os -mmcu=${ATMEGA}
CFLAGS=-g -Os -mmcu=${ATMEGA}

# avr-gcc -g -Os -mmcu=atmega644 -c demo.c
# avr-gcc -g -mmcu=atmega8 -o demo.elf demo.o
# avr-objcopy -j .text -j .data -O ihex demo.elf demo.hex
# sudo avrdude -p m644 -c avrispmkII -P usb -U demo.hex

clean:
	rm -f isp-flash-to-eeprom.o
	rm -f isp-flash-to-eeprom.elf
	rm -f isp-flash-to-eeprom.hex
	rm -f isp-flash-to-eeprom.map
	rm -f isp-flash-to-eeprom
	rm -f *~*
	rm -f _test_data.c _test_data.o uart.o uart_supplemental.o

_test_data.c:
	./data-to-c-hex.sh test_data.bin > _test_data.c

isp-flash-to-eeprom:	isp-flash-to-eeprom.o _test_data.o uart.o uart_supplemental.o
	avr-gcc -g -mmcu=${ATMEGA} -o isp-flash-to-eeprom.elf isp-flash-to-eeprom.o _test_data.o uart.o uart_supplemental.o

	avr-objcopy -j .text -j .data -O ihex isp-flash-to-eeprom.elf isp-flash-to-eeprom.hex

deploy:
	sudo avrdude -p ${ATMEGA} -c avrispmkII -P usb -U isp-flash-to-eeprom.hex
