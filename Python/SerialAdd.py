#import RPi.GPIO as GPIO
from time import sleep     # this lets us have a time delay (see line 12)

#Set a delay to allow for relays to mechanically switch
#Minimum time for a single relay propagation is 13ms
relaydelay = 0.02

#GPIO.setmode(GPIO.BCM)     # set up BCM GPIO numbering

#Set up the RPi ports that you wish to use
porta0 = 17
portb0 = 21
portc0 = 22

portsum0 = 23
portcarry0 = 24

#Setup Outputs
#GPIO.setup(porta0, GPIO.OUT)    # set GPIO 17 as output
#GPIO.setup(portb0, GPIO.OUT)    # set GPIO 21 as output (Revision 2 is 27)
#GPIO.setup(portc0, GPIO.OUT)    # set GPIO 22 as output

#Setup Inputs
#GPIO.setup(portsum0, GPIO.IN)    # set GPIO 23 as input
#GPIO.setup(portcarry0, GPIO.IN)    # set GPIO 24 as input

class relays:

    """A simple example class"""
    #i = 12345
    input_a = 0
    input_b = 0
    input_carry = 0

    #Full Adder Implementation
    def onebitadd(self):

        #print ("Input A: " + str(self.input_a) + " Input B: " + str(self.input_b) + " Input Carry: " + str(self.input_carry))

    #	rpi_output_sum = False
    #	rpi_output_carry = False

        #Set Full Adder Inputs
        #GPIO.output(porta0, input_a)
        #GPIO.output(portb0, input_b)
        #GPIO.output(portc0, input_carry)

        rpi_output_sum = self.input_a ^ self.input_b ^ self.input_carry
        rpi_output_carry = self.input_a and self.input_b or self.input_a and self.input_carry or self.input_b and self.input_carry

        #Wait for relays to switch
        sleep(relaydelay)

        #output_sum = GPIO.input(portsum0)
        #output_carry = GPIO.input(portcarry0)

        #if output_sum != rpi_output_sum or output_carry != rpi_output_carry:
            #print "Circuit failure outputting software calculated values"

        #print ("Output Sum: " + str(rpi_output_sum) + " Output Carry: " + str(rpi_output_carry))

        return (rpi_output_sum, rpi_output_carry)

    #Can handle negative inputs as long as the outcome it positive
    #This is because the bit length is arbitrary and the meaning of the MSB is ill defined
    #It would be correct if it was a interpreted as a two's complement word
    def serialunsignedadd(self, number1, number2):

        print "Input A:", str(number1), str(bin(number1))
        print "Input B:", str(number2), str(bin(number2))

        if number1.bit_length() > number2.bit_length():
            length = number1.bit_length()

        else:
            length = number2.bit_length()

        self.input_carry = 0    #Should never be required but can't hurt
        result = 0

        for bit in range(0, length+1):
            #length+1 is so we have an extra bit for overflow
            self.input_a = ((number1 & (2**bit))/(2**bit))
            self.input_b = ((number2 & (2**bit))/(2**bit))
            sum, self.input_carry = self.onebitadd()
            result += sum * 2**bit
            #print "Sum ", str(sum)
            #print "Carry ", str(self.input_carry)

        print "Result:", str(result), str(bin(result))
        return result

    def nbitserialadd(self, bitlength, number1, number2):

        self.input_carry = 0
        result = 0
        mask = 2**(bitlength-1)-1
        msbmask = 2**(bitlength-1)


        for bit in range(0, bitlength):
            self.input_a = ((number1 & (2**bit))/(2**bit))
            self.input_b = ((number2 & (2**bit))/(2**bit))
            sum, self.input_carry = self.onebitadd()
            result += sum * 2**bit
            #print "Sum ", str(sum)
            #print "Carry ", str(self.input_carry)

        msb = result & msbmask
        result = mask & result
        result -= msb
        print "Result:", str(result), str(bin(result))
        return result

try:
    test = relays()
    test.nbitserialadd(6,100,-120)
    #test.serialunsignedadd(1,1)


except KeyboardInterrupt:
    print "Cleaning GPIO"
    #GPIO.cleanup()         # clean up after yourself

finally:
    print "Cleaning GPIO"
    #GPIO.cleanup()         # clean up after yourself

