import RPi.GPIO as GPIO
from time import sleep     # this lets us have a time delay (see line 12)
GPIO.setmode(GPIO.BCM)     # set up BCM GPIO numbering

#Setup Outputs
GPIO.setup(17, GPIO.OUT, initial=0)    # set GPIO 17 as output (IN A)
GPIO.setup(21, GPIO.OUT, initial=0)    # set GPIO 21 as output (IN B) (Revision 2 is 27)
GPIO.setup(22, GPIO.OUT, initial=0)    # set GPIO 22 as output (CARRY IN)

#Setup Inputs
GPIO.setup(23, GPIO.IN)    # set GPIO 23 as input (SUM)
GPIO.setup(24, GPIO.IN)    # set GPIO 24 as input (CARRY OUT)

#Variables
CorrectStates = 0
SleepTime = 0.1

try:
  	#Try state A=0, B=0, CARRY IN=0
	print "Testing A=0, B=0, CarryIn=0"
	GPIO.output(17, 0)
	GPIO.output(21, 0)
	GPIO.output(22, 0)

	sleep(SleepTime)

        if GPIO.input(23)==0 and GPIO.input(24)==0: # SUM 0, CARRY OUT 0
		print "Output is 00 (Match)"
		CorrectStates += 1
	
        if GPIO.input(23)==1 and GPIO.input(24)==0: # SUM 1, CARRY OUT 0
		print "Output is 01 (Error)"

        if GPIO.input(23)==0 and GPIO.input(24)==1: # SUM 0, CARRY OUT 1
		print "Output is 10 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==1: # SUM 1, CARRY OUT 1
		print "Output is 11 (Error)"

        #Try state A=0, B=0, CARRY IN=1
	print ("Testing A=0, B=0, CarryIn=1")
        GPIO.output(17, 0)
        GPIO.output(21, 0)
        GPIO.output(22, 1)

	sleep(SleepTime)

        if GPIO.input(23)==0 and GPIO.input(24)==0: # SUM 0, CARRY OUT 0
                print "Output is 00 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==0: # SUM 1, CARRY OUT 0
                print "Output is 01 (Match)"
		CorrectStates += 1

        if GPIO.input(23)==0 and GPIO.input(24)==1: # SUM 0, CARRY OUT 1
                print "Output is 10 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==1: # SUM 1, CARRY OUT 1
                print "Output is 11 (Error)"

        #Try state A=0, B=1, CARRY IN=0
	print ("Testing A=0, B=1, CarryIn=0")
        GPIO.output(17, 0)
        GPIO.output(21, 1)
        GPIO.output(22, 0)

        sleep(SleepTime)

        if GPIO.input(23)==0 and GPIO.input(24)==0: # SUM 0, CARRY OUT 0
                print "Output is 00 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==0: # SUM 1, CARRY OUT 0
                print "Output is 01 (Match)"
                CorrectStates += 1

        if GPIO.input(23)==0 and GPIO.input(24)==1: # SUM 0, CARRY OUT 1
                print "Output is 10 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==1: # SUM 1, CARRY OUT 1
                print "Output is 11 (Error)"

        #Try state A=1, B=0, CARRY IN=0
	print ("Testing A=1, B=0, CarryIn=0")
        GPIO.output(17, 1)
        GPIO.output(21, 0)
        GPIO.output(22, 0)

        sleep(SleepTime)

        if GPIO.input(23)==0 and GPIO.input(24)==0: # SUM 0, CARRY OUT 0
                print "Output is 00 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==0: # SUM 1, CARRY OUT 0
                print "Output is 01 (Match)"
                CorrectStates += 1

        if GPIO.input(23)==0 and GPIO.input(24)==1: # SUM 0, CARRY OUT 1
                print "Output is 10 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==1: # SUM 1, CARRY OUT 1
                print "Output is 11 (Error)"

        #Try state A=0, B=1, CARRY IN=1
	print ("Testing A=0, B=1, CarryIn=1")
        GPIO.output(17, 0)
        GPIO.output(21, 1)
        GPIO.output(22, 1)

	sleep(SleepTime)

        if GPIO.input(23)==0 and GPIO.input(24)==0: # SUM 0, CARRY OUT 0
                print "Output is 00 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==0: # SUM 1, CARRY OUT 0
                print "Output is 01 (Error)"

        if GPIO.input(23)==0 and GPIO.input(24)==1: # SUM 0, CARRY OUT 1
                print "Output is 10 (Match)"
		CorrectStates += 1

        if GPIO.input(23)==1 and GPIO.input(24)==1: # SUM 1, CARRY OUT 1
                print "Output is 11 (Error)"

        #Try state A=1, B=0, CARRY IN=1
	print ("Testing A=1, B=0, CarryIn=1")
        GPIO.output(17, 1)
        GPIO.output(21, 0)
        GPIO.output(22, 1)

        sleep(SleepTime)

        if GPIO.input(23)==0 and GPIO.input(24)==0: # SUM 0, CARRY OUT 0
                print "Output is 00 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==0: # SUM 1, CARRY OUT 0
                print "Output is 01 (Error)"

        if GPIO.input(23)==0 and GPIO.input(24)==1: # SUM 0, CARRY OUT 1
                print "Output is 10 (Match)"
                CorrectStates += 1

        if GPIO.input(23)==1 and GPIO.input(24)==1: # SUM 1, CARRY OUT 1
                print "Output is 11 (Error)"

        #Try state A=1, B=1, CARRY IN=0
	print ("Testing A=1, B=1, CarryIn=0")
        GPIO.output(17, 1)
        GPIO.output(21, 1)
        GPIO.output(22, 0)

        sleep(SleepTime)

        if GPIO.input(23)==0 and GPIO.input(24)==0: # SUM 0, CARRY OUT 0
                print "Output is 00 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==0: # SUM 1, CARRY OUT 0
                print "Output is 01 (Error)"

        if GPIO.input(23)==0 and GPIO.input(24)==1: # SUM 0, CARRY OUT 1
                print "Output is 10 (Match)"
                CorrectStates += 1

        if GPIO.input(23)==1 and GPIO.input(24)==1: # SUM 1, CARRY OUT 1
                print "Output is 11 (Error)"

        #Try state A=1, B=1, CARRY IN=1
	print ("Testing A=1, B=1, CarryIn=1")
        GPIO.output(17, 1)
        GPIO.output(21, 1)
        GPIO.output(22, 1)

	sleep(SleepTime)
	
        if GPIO.input(23)==0 and GPIO.input(24)==0: # SUM 0, CARRY OUT 0
                print "Output is 00 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==0: # SUM 1, CARRY OUT 0
                print "Output is 01 (Error)"

        if GPIO.input(23)==0 and GPIO.input(24)==1: # SUM 0, CARRY OUT 1
                print "Output is 10 (Error)"

        if GPIO.input(23)==1 and GPIO.input(24)==1: # SUM 1, CARRY OUT 1
                print "Output is 11 (Match)"
		CorrectStates += 1

	#Analyse Number of Correct States
	print ("Number of correct states is " + str(CorrectStates) + " out of 8")

	if CorrectStates == 8:
		print "Functionality Correct"
	else:
		print "Functionality Incorrect"

except KeyboardInterrupt:
	print "Interrupted By User"

except:
	print "Exception Occured"	

finally:
	print "Turning off outputs"
	GPIO.output(17, 0)
        GPIO.output(21, 0)
        GPIO.output(22, 0)
	print "Cleaning Up GPIO"
	GPIO.cleanup()
