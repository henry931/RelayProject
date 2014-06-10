#include <vector>
#include <stdint.h>
#include <stdio.h>

// Pi Headers

#include <wiringPi.h>
#include <mcp23017.h>
#include <unistd.h>


class ALU {

	// Critical delay in seconds to allow relays to switch 
	static const unsigned ClockPeriod = unsigned(10000);

	// Serial IO Base Pins
	static const unsigned IC1 = 100;
	static const unsigned IC2 = 200;

	// Mapped input pins
	static const unsigned ADDR_A[8], ADDR_B[8], ADDR_R[8], ADDR_I[3], ADDR_Carry;

public:

	// Variables for input and output
	uint8_t A, B, Instruction, Result, CarryOut;

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
		usleep(ClockPeriod);

		// Read In Result
		Result = 0;
		for (unsigned i = 0; i < 8; i++) Result |= digitalRead(ADDR_R[i]) << i;
		CarryOut = digitalRead(ADDR_Carry);
		
		return 0;

	}
};

// Set pin ordering here - depends on PCB
const unsigned ALU::ADDR_A[8] = { IC2 + 3, IC2 + 2, IC2 + 5, IC2 + 4, IC1 + 12, IC1 + 11, IC2 + 1, IC2 + 0 };
const unsigned ALU::ADDR_B[8] = { IC1 + 8, IC1 + 7, IC1 + 10, IC1 + 9, IC1 + 4, IC1 + 3, IC1 + 6, IC1 + 5 };
const unsigned ALU::ADDR_R[8] = { IC2 + 8, IC2 + 7, IC2 + 6, IC2 + 11, IC2 + 10, IC2 + 9, IC2 + 14, IC2 + 13 };
const unsigned ALU::ADDR_I[3] = { IC1 + 1, IC1 + 0, IC1 + 2 };
const unsigned ALU::ADDR_Carry = IC2 + 12;

int GetInstruction(void)
{
	return 0;
}

int TemplateInstruction( ALU *RelayALU, void (ALU::*ClockALU)(), std::vector<uint16_t> &MEMORY, uint16_t &address)
{

	// Run instruction through ALU
	((RelayALU)->*(ClockALU))();

	return 0;
}

int main(void)
{
	// Processor memory 12-bit addressing
	std::vector<uint16_t> MEMORY(4096);

	// Finished Variable
	unsigned finished = 0;

	// Registers
	// IR - Current Instruction
	// ACC - Accumulator
	// PC - Current Address
	uint16_t IR = 0, ACC = 0, PC = 0;

	// Instruction to execute	
	uint16_t address = 0, opcode = 0;

	ALU RelayALU;
	RelayALU.SetupInterface();

	//Parse assembly text file and load memory

	RelayALU.A = 0;
	RelayALU.B = 0;
	RelayALU.Instruction = 0;

	while(1)
	{
		if(RelayALU.CarryOut != 1)
		{
		RelayALU.A = 255;
		RelayALU.B = 255;
		RelayALU.Instruction = 7;
		}
		else
		{
		RelayALU.A = 0;
                RelayALU.B = 0;
                RelayALU.Instruction = 0;
		}
		RelayALU.ClockALU();
	}
	while(1)
	{
		for(unsigned a = 0; a <= 256; a++)
		{
			RelayALU.A = a;
			RelayALU.ClockALU();
			//RelayALU.A = 0;
                        //RelayALU.ClockALU();
		}

		for(unsigned a = 0; a <= 256; a++)
                {
                        RelayALU.B = a;
                        RelayALU.ClockALU();
                        //RelayALU.A = 0;
                        //RelayALU.ClockALU();
                }

		 for(unsigned a = 0; a <= 8; a++)
                {
                        RelayALU.Instruction = a;
                        RelayALU.ClockALU();
                        //RelayALU.A = 0;
                        //RelayALU.ClockALU();
                }

		//for(unsigned b = 0; b < 256; b++)
                //{
                //}
	}

	while (!finished)
	{
		// Get instruction
		IR = MEMORY[PC];

		// Extract opcode & address
		opcode = IR >> 12;
		address = IR & 4095;

		//Execute Instruction
		switch (opcode) {
		case 0: break;
		case 1: break;
		case 2: break;
		case 3: break;
		case 4: break;
		case 5: break;
		case 6: break;
		case 7: break;
		case 8: break;
		case 9: break;
		case 10: break;
		case 11: break;
		case 12: break;
		case 13: break;
		case 14: break;
		case 15: break;
		default:
			// Code
			break;
		}

		// Increment program counter
		PC++;
	}

	return 0;
}
