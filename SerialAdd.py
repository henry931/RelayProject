import RPi.GPIO as GPIO
from time import sleep     # this lets us have a time delay (see line 12)
GPIO.setmode(GPIO.BCM)     # set up BCM GPIO numbering

#Setup Outputs
GPIO.setup(17, GPIO.OUT)    # set GPIO 17 as output
GPIO.setup(21, GPIO.OUT)    # set GPIO 21 as output (Revision 2 is 27)
GPIO.setup(22, GPIO.OUT)    # set GPIO 22 as output

#Setup Inputs
GPIO.setup(23, GPIO.IN)    # set GPIO 23 as input
GPIO.setup(24, GPIO.IN)    # set GPIO 24 as input

try:
    while True:            # this will carry on until you hit CTRL+C

        if GPIO.input(23): # if port 25 == 1
            print "Port 23 is 1/GPIO.HIGH/True"
        else:
            print "Port 23 is 0/GPIO.LOW/False"

        if GPIO.input(24): # if port 24 == 1
            print "Port 24 is 1/GPIO.HIGH/True"
        else:
            print "Port 24 is 0/GPIO.LOW/False"        

	sleep(1)         # wait 1 seconds

except KeyboardInterrupt:
    GPIO.cleanup()         # clean up after yourself
