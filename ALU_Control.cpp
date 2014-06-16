#include <vector>
#include <cstdint>
//#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <bitset>

// Pi Headers
#ifdef __ARMEL__
#include <wiringPi.h>
#include <mcp23017.h>
#include <unistd.h>
#endif

#define MEMORY_LEN 4096

// Information level
static int verbose;

class ALU {

	// Critical delay in microseconds to allow relays to switch 
	static const unsigned ClockPeriod = unsigned(200*1000);

	// Serial IO Base Pins
	static const unsigned IC1 = 100;
	static const unsigned IC2 = 200;

	// Mapped input pins
	static const unsigned ADDR_A[8], ADDR_B[8], ADDR_R[8], ADDR_I[3], ADDR_Carry;

public:

	// Variables for input and output
	uint8_t A, B, Instruction, Result, CarryOut;

	ALU() : A(0), B(0), Instruction(0), Result(0), CarryOut(0)
	{
	}

	int SetupInterface(void)
	{
#ifdef __ARMEL__

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

#endif
		return 0;
	}

	int ClockALU(void)
	{
#ifdef __ARMEL__

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
		// Inputs are active high so invert
		for (unsigned i = 0; i < 8; i++) Result |= (!digitalRead(ADDR_R[i])) << i;
		CarryOut = !digitalRead(ADDR_Carry);

#endif
		return 0;
	}

	int SimulateALU(void)
	{
		uint16_t tempResult = 0;

		switch (Instruction) {
			// Add
		case 0:
			tempResult = A + B;
			Result = uint8_t(tempResult);
			CarryOut = uint8_t(tempResult >> 8);
			break;
			// Inc
		case 1:
			tempResult = A + 1;
			Result = uint8_t(tempResult);
			CarryOut = uint8_t(tempResult >> 8);
			break;
			// Xor
		case 2:
			Result = A ^ B;
			CarryOut = (A >> 7) & (B >> 7);
			break;
			// And
		case 3:
			Result = A & B;
			CarryOut = (A >> 7) & (B >> 7);
			break;
			// Xnor
		case 4:
			Result = ~(A ^ B);
			CarryOut = (A >> 7) | (B >> 7);
			break;
			// Or
		case 5:
			Result = A | B;
			CarryOut = (A >> 7) | (B >> 7);
			break;
			// Not
		case 6:
			Result = ~A;
			CarryOut = (A >> 7);
			break;
			// Rol
		case 7:
			Result = (A << 1) | (A >> 7);
			CarryOut = (A >> 7);
			break;
		default:
			// Code
			break;
		}
	}

	int ClockTestALU(void)
	{
		// Test in hardware
		ClockALU();

		const uint8_t
			Temp_Result = Result,
			Temp_CarryOut = CarryOut;

		// Test in software
		SimulateALU();

#ifndef __ARMEL__
		return 0;
#endif

		// Check results match
		if (Result != Temp_Result)
		{
			const std::bitset<8> expected_bits(Result), actual_bits(Temp_Result), a_bits(A), b_bits(B), instruction_bits(Instruction);
			std::cout << "Error:       Result" << std::endl;
			std::cout << "Expected:    " << expected_bits << std::endl;
			std::cout << "Actual:      " << actual_bits << std::endl;
			std::cout << "A:           " << a_bits << std::endl;
			std::cout << "B:           " << b_bits << std::endl;
			std::cout << "Instruction: " << instruction_bits << std::endl;
			std::cout << std::endl;
			return -1;
		}

		if (CarryOut != Temp_CarryOut)
		{
			const std::bitset<8> expected_bits(CarryOut), actual_bits(Temp_CarryOut), a_bits(A), b_bits(B), instruction_bits(Instruction);
			std::cout << "Error:       Result" << std::endl;
			std::cout << "Expected:    " << expected_bits << std::endl;
			std::cout << "Actual:      " << actual_bits << std::endl;
			std::cout << "A:           " << a_bits << std::endl;
			std::cout << "B:           " << b_bits << std::endl;
			std::cout << "Instruction: " << instruction_bits << std::endl;
			std::cout << std::endl;
			return -1;
		}

		if (verbose = 1)
		{
			const std::bitset<8> result_bits(Result), carry_bits(CarryOut), a_bits(A), b_bits(B), instruction_bits(Instruction);
			std::cout << "OK" << std::endl;
			std::cout << "Result:      " << result_bits << std::endl;
			std::cout << "Carry:       " << carry_bits << std::endl;
			std::cout << "A:           " << a_bits << std::endl;
			std::cout << "B:           " << b_bits << std::endl;
			std::cout << "Instruction: " << instruction_bits << std::endl;
			std::cout << std::endl;
		}
		return 0;
	}
};

// Set pin ordering here - depends on PCB
// First eight physical IC outputs are software pins 0 thru 7
// Second eight physical IC outputs are software pins 8 thru 15
const unsigned ALU::ADDR_A[8] = { IC2 + 3, IC2 + 2, IC2 + 5, IC2 + 4, IC1 + 12, IC1 + 11, IC2 + 1, IC2 + 0 };
const unsigned ALU::ADDR_B[8] = { IC1 + 8, IC1 + 7, IC1 + 10, IC1 + 9, IC1 + 4, IC1 + 3, IC1 + 6, IC1 + 5 };
const unsigned ALU::ADDR_R[8] = { IC2 + 8, IC2 + 7, IC2 + 6, IC2 + 11, IC2 + 10, IC2 + 9, IC2 + 14, IC2 + 13 };
const unsigned ALU::ADDR_I[3] = { IC1 + 1, IC1 + 0, IC1 + 2 };
const unsigned ALU::ADDR_Carry = IC2 + 12;

int LoadProgram(const std::vector<uint16_t> &MEMORY, const std::string &SourceFile)
{
	typedef struct {
		size_t starts;
		size_t ends;
	} argument_t;

	// Labelled indicated a label is used as to indicate the address
	// Instruction indicates that the line is an instruction
	std::vector<bool> Labelled(MEMORY_LEN, 0), Instruction(MEMORY_LEN, 0);

	// Isolate specific mnemonics for binary conversion
	std::string line, label, opcode, destination;

	unsigned linecount{};

	std::ifstream ifs(SourceFile);
	if (!ifs.is_open())
	{
		std::cout << "Error: Could not open file" << std::endl;
		return -1;
	}

	// Keep getting lines until EOF
	while (getline(ifs, line))
	{

		// Find whitespace
		const size_t linelength = line.length();
		std::vector<bool> whitespace(linelength, 1);
		for (size_t i{}; i < linelength; i++) if (line.compare(i, 1, " ", 1)) whitespace[i] = 0;

		// Find arguments
		bool midargument{};
		//unsigned argumentsfound{};
		std::vector<argument_t> arguments;
		// First char
		if (!whitespace[0])
		{
			// Found argument
			midargument = 1;
			arguments.push_back(argument_t{ 0 });
		}
		// Mid chars
		for (size_t i = 1; i < linelength; i++)
		{
			// Argument begins at i
			if (whitespace[i - 1] & (!whitespace[i]))
			{
				midargument = 1;
				arguments.push_back(argument_t{ i });
			}

			// Argument ends at i - 1
			if ((!whitespace[i - 1]) & whitespace[i])
			{
				midargument = 0;
				arguments.back().ends = i-1;
			}
		}
		//Last char
		if (midargument) arguments.back().ends = linelength - 1;

		const std::size_t colonpos = line.find(':');
		if (colonpos == size_t(-1))
		{
			// No label found
		}
		else
			
		{
			// Label found
			Labelled[linecount] = 1;

			for (unsigned i = 0; i < colonpos; i++) int hello;
		}

		linecount++;
	}

	ifs.close();
	return 0;
}

int GetInstruction(void)
{
	return 0;
}

int TemplateInstruction(ALU *RelayALU, void (ALU::*ClockALU)(), std::vector<uint16_t> &MEMORY, uint16_t &address)
{

	// Run instruction through ALU
	((RelayALU)->*(ClockALU))();

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc>1)
	{
		try { verbose = std::stoi(argv[1]); }
		catch (const std::invalid_argument &ia) { verbose = 0; }
	}

	// Processor memory 12-bit addressing
	std::vector<uint16_t> MEMORY(MEMORY_LEN, 0);

	// Finished Variable
	bool finished = 0;

	// Registers
	// IR - Current Instruction
	// ACC - Accumulator
	// PC - Current Address
	uint16_t IR = 0, ACC = 0, PC = 0;

	// Instruction to execute	
	uint16_t address = 0, opcode = 0;

	// Set up interface
	ALU RelayALU;
	RelayALU.SetupInterface();

	// Test loop
	        for(size_t iin{0}; iin < 8; iin ++)
        {
                RelayALU.Instruction = iin;
                
                for(size_t ain{}; ain < 256; ain ++)
                {
                         RelayALU.A = ain;

                        for(size_t bin{}; bin < 256; bin ++)
                        {
                                RelayALU.B = bin;

                                RelayALU.ClockTestALU();
                        }             
                }
        }

	// Parse assembly text file and load memory
	const std::string SourceFile = "input.txt";
	LoadProgram(MEMORY, SourceFile);

	while ((!finished) && (PC < MEMORY_LEN))
	{
		// Get instruction
		IR = MEMORY[PC];

		// Extract opcode & address
		opcode = IR >> 12;
		address = IR & (MEMORY_LEN-1);

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
