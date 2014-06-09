#include <vector>
#include <stdint.h>
#include <stdio.h>

// Pi Headers
/*
#include <wiringPi.h>
#include <mcp23017.h>
#include <unistd.h>
*/

class ALU {

	// Critical delay in seconds to allow relays to switch 
	unsigned ClockPeriod = 0.5;

	// Variables for input and output
	uint8_t A, B, Instruction, Result, CarryOut;

	// Serial IO Base Pins
	static const unsigned IC1 = 100;
	static const unsigned IC2 = 200;

	// Mapped input pins
	static const unsigned ADDR_A[8], ADDR_B[8], ADDR_R[8], ADDR_I[3], ADDR_Carry;
	
	int SetupInterface(void)
	{
		// WiringPi setup
		wiringPiSetup();

		// Serial IO IC setup
		mcp23017Setup(IC1, 0x20);
		mcp23017Setup(IC2, 0x21);
		
		// Set IO modes, pull-ups etc
		for (unsigned i = 0; i < 8; i++)
		{
			pinMode(ADDR_A[i], OUTPUT);
			pinMode(ADDR_B[i], OUTPUT);
			pinMode(ADDR_R[i], INPUT);
			pullUpDnControl(ADDR_R[i], PUD_UP);
		}

		for (unsigned i = 0; i < 3; i++)
		{
			pinMode(ADDR_I[i], OUTPUT);
		}

		pinMode(ADDR_Carry, INPUT);
		pullUpDnControl(ADDR_Carry, PUD_UP);

		return 0;
	}
	
	int ClockALU(void)
	{
		// Write to outputs
		// A & B
		for (unsigned i = 0; i < 8; i++)
		{
			digitalWrite(ADDR_A[i], A & (1 << i));
			digitalWrite(ADDR_B[i], B & (1 << i));
		}
		// Instruction
		for (unsigned i = 0; i < 3; i++) digitalWrite(ADDR_I[i], Instruction & (1 << i));

		// Wait for relays
		sleep(ClockPeriod);

		// Read In Result
		Result = 0;
		for (unsigned i = 0; i < 8; i++) Result |= digitalRead(ADDR_R[i]) << i;
		CarryOut = digitalRead(ADDR_Carry);

		return 0;
	}
};

// Set pin ordering here - depends on PCB
const unsigned ALU::ADDR_A[8] = { IC2 + 3, IC2 + 2, IC2 + 5, IC2 + 4, IC1 + 12, IC1 + 11, IC2 + 1, IC2 + 0 };
const unsigned ALU::ADDR_B[8] = { IC2 + 3, IC2 + 2, IC2 + 5, IC2 + 4, IC1 + 12, IC1 + 11, IC2 + 1, IC2 + 0 };
const unsigned ALU::ADDR_R[8] = { IC2 + 8, IC2 + 7, IC2 + 6, IC2 + 11, IC2 + 10, IC2 + 9, IC2 + 14, IC2 + 13 };
const unsigned ALU::ADDR_I[3] = { IC1 + 1, IC1 + 0, IC1 + 2 };
const unsigned ALU::ADDR_Carry = IC2 + 12;


int main(void)
{
	std::vector<uint16_t> MEMORY;

	return 0;
}