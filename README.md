zikburner
=========

Minimal eeprom programmer/burner based on atmega mcu; target devices are the 28Cxxx line of eeproms.
Example include AT28C256 (256kbit or 32K) eeprom from Atmel. Can also target 128kbit, 64kbit etc..
any of the 28pin line of eeproms.

Included schematic targets serial over USB (via an FTDI chip, as commonly used by Arduino); it is
trivial to substitute in a MAX232 to drive an actual PC serial port (the MAX chip will adjust
voltage levels to match PC requirements.)
