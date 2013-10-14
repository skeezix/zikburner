#!/usr/bin/python

import sys

def readline():
    b = None
    while True:
        c = ser.read()
        if c:
            if c == '\r':
                pass
            elif c == '\n':
                break
            else:
                if b == None:
                    b = c
                else:
                    b += c
        else:
            break

    #print "received:", b
    return b

def wait_for_ready():
    while True:
        b = readline()
        if b == '+READY':
            break
        #print "swallow", b
    return True

def send ( b ):
    #print "Sending", b
    ser.write ( b )

# command line args
#

from optparse import OptionParser

cmdparser = OptionParser()
cmdparser.add_option ( "-a", "--address", dest="address", help="address to begin dumping", default=None )
cmdparser.add_option ( "-l", "--length", dest="length",   help="length of dump", default=None )
cmdparser.add_option ( "-d", "--device", dest="device",   help="device name (fully qualified path)", default=None )

(cmdoptions, cmdargs) = cmdparser.parse_args()

# dump argument summary
#

print "Start address:\t", cmdoptions.address
print "Length of dump:\t", cmdoptions.length
print "Port device:\t", cmdoptions.device

if not cmdoptions.address or not cmdoptions.length or not cmdoptions.device:
    print "Missing arguments; need address, length, portname"
    sys.exit()

# do it
#

import serial
ser = serial.Serial ( port = cmdoptions.device, baudrate = 9600, timeout = 2.05, rtscts = True )

if not ser:
    print "Couldn't open serial port", cmdoptions.device
    sys.exit()

send ( "ohai\r" )
wait_for_ready()

send ( "charecho\r" )
wait_for_ready()

send ( "format\r" )
wait_for_ready()

send ( "dump " + cmdoptions.address + " " + cmdoptions.length + "\r" )
while True:
    b = readline()
    if b:
        ( a, v ) = b.split ( ':', 1 )
        a = a.strip()
        v = int ( v.strip(), 16 )

        print a + ":", v

    else:
        break

# done
#

ser.close()
sys.exit()
