#include <vector>
#include <cstdint>
//#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <stdexcept>
#include <ctype.h>
#include <iomanip>
#include <algorithm>

// Pi Headers
#ifdef __ARMEL__
#include <wiringPi.h>
#include <mcp23017.h>
#include <unistd.h>
#include <term.h>

// From: http://www.cplusplus.com/articles/4z18T05o/
void clearscreen()
{
	if (!cur_term)
	{
		int result;
		setupterm(NULL, STDOUT_FILENO, &result);
		if (result <= 0) return;
	}

	putp(tigetstr("clear"));
}
#endif

// Windows
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#include <limits>
// From: http://www.cplusplus.com/articles/4z18T05o/
// To avoid system calls
void clearscreen()
{
	HANDLE                     hStdOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD                      count;
	DWORD                      cellCount;
	COORD                      homeCoords = { 0, 0 };

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == INVALID_HANDLE_VALUE) return;

	/* Get the number of cells in the current buffer */
	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
	cellCount = csbi.dwSize.X *csbi.dwSize.Y;

	/* Fill the entire buffer with spaces */
	if (!FillConsoleOutputCharacter(
		hStdOut,
		(TCHAR) ' ',
		cellCount,
		homeCoords,
		&count
		)) return;

	/* Fill the entire buffer with the current colors and attributes */
	if (!FillConsoleOutputAttribute(
		hStdOut,
		csbi.wAttributes,
		cellCount,
		homeCoords,
		&count
		)) return;

	/* Move the cursor home */
	SetConsoleCursorPosition(hStdOut, homeCoords);
}
#endif

// http://www.cplusplus.com/forum/beginner/1988/
void pause()
{
	std::cout << "Press ENTER to continue...";
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

#define MEMORY_LEN 4096
#define LOWHALF 255
#define HIGHHALF 65280
#define MU0MAXWORD 65535

// Information level
static int verbose;

class ALU {

	// Serial IO Base Pins
	static const unsigned IC1 = 100;
	static const unsigned IC2 = 200;

	// Mapped input pins
	static const unsigned ADDR_A[8], ADDR_B[8], ADDR_R[8], ADDR_I[3], ADDR_Carry;

public:

	// Critical delay in microseconds to allow relays to switch 
	unsigned ClockPeriod;

	// Variables for input and output
	uint8_t A, B, Instruction, Result, CarryOut;

	ALU() : A(0), B(0), Instruction(0), Result(0), CarryOut(0), ClockPeriod{ 100000 }
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

		// Simulated equivalents of relay ALU
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

		return 0;
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

		// and carry
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

		// Print information depending on debug level
		if (verbose > 0)
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

int LoadProgram(std::vector<uint16_t> &MEMORY, const std::string &SourceFile)
{
	typedef struct {
		bool LabelDefinition;
		bool LabelledInstruction;
		bool PlainInstruction;
		// Opcode & corresponding mnemonic
		uint16_t Opcode;
		std::string Mnemonic;
		// Address & corresponding label
		uint16_t Address;
		std::string Label;
		// Literal and corresponding destination
		uint16_t Literal;
		std::string Destination;
	} buffer_mem_t;

	typedef struct {
		size_t starts;
		size_t ends;
	} argument_t;

	std::vector<buffer_mem_t> MemoryBuff;

	// Isolate specific mnemonics for binary conversion
	std::string line, label, opcode, destination;

	std::ifstream ifs(SourceFile);
	if (!ifs.is_open())
	{
		std::cout << "Error: Could not open file" << std::endl;
		return -1;
	}

	// Keep getting lines until EOF
	while (getline(ifs, line))
	{
		//Remove windows line endings for Pi
		if (line.back() == 0x0D) line.pop_back();

		// Find whitespace
		const size_t linelength = line.length();
		std::vector<bool> whitespace(linelength, 1);
		for (size_t i{}; i < linelength; i++) if (line.compare(i, 1, " ", 1)) whitespace[i] = 0;

		// Find arguments by identifying whitespace boundaries
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
				arguments.back().ends = i - 1;
			}
		}
		//Last char
		if (midargument) arguments.back().ends = linelength - 1;

		const std::size_t colonpos = line.find(':');
		if (colonpos != line.rfind(':'))
		{
			// Complain
			std::wcout << "Error: More than one colon in line " << MemoryBuff.size() << std::endl;
			return -1;
		}
		if (colonpos == size_t(-1))
		{
			// No label found...
			// Must be a plain instruction

			// Add to memory buffer
			// First argument is mnemonic
			// Second argument is destination
			if (arguments.size() == 2)
			{
				MemoryBuff.push_back(buffer_mem_t{ 0, 0, 1 }); // See struct definition
				MemoryBuff.back().Mnemonic = line.substr(arguments[0].starts, arguments[0].ends - arguments[0].starts + 1);
				MemoryBuff.back().Destination = line.substr(arguments[1].starts, arguments[1].ends - arguments[1].starts + 1);
			}
			else if (arguments.size() == 1)
			{
				MemoryBuff.push_back(buffer_mem_t{ 0, 0, 1 }); // See struct definition
				MemoryBuff.back().Mnemonic = line.substr(arguments[0].starts, arguments[0].ends - arguments[0].starts + 1);
			}
			else
			{
				// Must have one or two arguments
				std::wcout << "Error: Bad number of arguments in line " << MemoryBuff.size() << std::endl;
				return -1;
			}

		}
		else
		{
			// Label found

			if ((colonpos != arguments[0].ends) && (colonpos != arguments[1].starts))
			{
				// Colon in forbidden position
				std::wcout << "Error: Colon in bad position in line " << MemoryBuff.size() << std::endl;
				return -1;
			}

			//Remove colon reference
			if (colonpos == arguments[0].ends) arguments[0].ends--;
			else if (colonpos == arguments[1].starts)
			{
				// if argument just a colon - delete, else increment
				if (arguments[1].starts == arguments[1].ends) arguments.erase(arguments.begin() + 1);
				else arguments[1].starts++;
			}

			if (arguments.size() == 2)
			{
				// Label definition OR Instruction without destination
				// Decide by seeing if argument two is a number or not
				bool isnumber = 1;
				for (size_t iter = arguments[1].starts; iter <= arguments[1].ends; iter++)
				{
					if (!isdigit(line[iter]))
					{
						isnumber = 0;
						break;
					}
				}

				// Label definition
				if (isnumber)
				{
					MemoryBuff.push_back(buffer_mem_t{ 1, 0, 0 });
					MemoryBuff.back().Label = line.substr(arguments[0].starts, arguments[0].ends - arguments[0].starts + 1);
					MemoryBuff.back().Destination = line.substr(arguments[1].starts, arguments[1].ends - arguments[1].starts + 1);
				}
				// Instruction with label (without destination)
				else
				{
					// Instruction with label
					MemoryBuff.push_back(buffer_mem_t{ 0, 1, 0 });
					MemoryBuff.back().Label = line.substr(arguments[0].starts, arguments[0].ends - arguments[0].starts + 1);
					MemoryBuff.back().Mnemonic = line.substr(arguments[1].starts, arguments[1].ends - arguments[1].starts + 1);
				}

			}
			else if (arguments.size() == 3)
			{
				// Instruction with label
				MemoryBuff.push_back(buffer_mem_t{ 0, 1, 0 });
				MemoryBuff.back().Label = line.substr(arguments[0].starts, arguments[0].ends - arguments[0].starts + 1);
				MemoryBuff.back().Mnemonic = line.substr(arguments[1].starts, arguments[1].ends - arguments[1].starts + 1);
				MemoryBuff.back().Destination = line.substr(arguments[2].starts, arguments[2].ends - arguments[2].starts + 1);
			}
			else
			{
				std::wcout << "Error: Bad number of instruction in line " << MemoryBuff.size() << std::endl;
				return -1;
			}
		}
	}

	// Close file
	ifs.close();

	// Process Memory Buffer	
	for (size_t iter{}; iter < MemoryBuff.size(); iter++)
	{
		// Instructions can be converted straight to binary
		if (!MemoryBuff[iter].Mnemonic.compare("LDA")) MemoryBuff[iter].Opcode = 0;
		else if (!MemoryBuff[iter].Mnemonic.compare("STO")) MemoryBuff[iter].Opcode = 1;
		else if (!MemoryBuff[iter].Mnemonic.compare("ADD")) MemoryBuff[iter].Opcode = 2;
		else if (!MemoryBuff[iter].Mnemonic.compare("SUB")) MemoryBuff[iter].Opcode = 3;
		else if (!MemoryBuff[iter].Mnemonic.compare("JMP")) MemoryBuff[iter].Opcode = 4;
		else if (!MemoryBuff[iter].Mnemonic.compare("JGE")) MemoryBuff[iter].Opcode = 5;
		else if (!MemoryBuff[iter].Mnemonic.compare("JNE")) MemoryBuff[iter].Opcode = 6;
		else if (!MemoryBuff[iter].Mnemonic.compare("STP")) MemoryBuff[iter].Opcode = 7;
		else if (!MemoryBuff[iter].Mnemonic.compare("MUL")) MemoryBuff[iter].Opcode = 8;
		else if (!MemoryBuff[iter].Mnemonic.compare("INC")) MemoryBuff[iter].Opcode = 9;
		else if (!MemoryBuff[iter].Mnemonic.compare("XOR")) MemoryBuff[iter].Opcode = 10;
		else if (!MemoryBuff[iter].Mnemonic.compare("AND")) MemoryBuff[iter].Opcode = 11;
		else if (!MemoryBuff[iter].Mnemonic.compare("XNR")) MemoryBuff[iter].Opcode = 12;
		else if (!MemoryBuff[iter].Mnemonic.compare("LOR")) MemoryBuff[iter].Opcode = 13;
		else if (!MemoryBuff[iter].Mnemonic.compare("NOT")) MemoryBuff[iter].Opcode = 14;
		else if (!MemoryBuff[iter].Mnemonic.compare("ROL")) MemoryBuff[iter].Opcode = 15;

		// Fill in adresses
		MemoryBuff[iter].Address = uint16_t(iter);
	}

	// Match up labels and convert to addresses
	for (size_t iter{}; iter < MemoryBuff.size(); iter++)
	{
		std::cout << iter << std::endl;
		// If an instruction
		if (MemoryBuff[iter].LabelledInstruction || MemoryBuff[iter].PlainInstruction)
		{
			// If destinationless opcode, just set literal to zero
			if ((MemoryBuff[iter].Opcode == 7) ||
				(MemoryBuff[iter].Opcode == 9) ||
				(MemoryBuff[iter].Opcode == 14) ||
				(MemoryBuff[iter].Opcode == 15))
				// Do this
				MemoryBuff[iter].Literal = 0;
			else
			{
				// Try to convert to int (test for literal)
				try
				{
					MemoryBuff[iter].Literal = stoi(MemoryBuff[iter].Destination);
				}
				catch (const std::invalid_argument &ia)
				{
					std::cout << "Not a literal. Looking for label... " << ia.what() << std::endl;
					for (size_t inner{}; inner < MemoryBuff.size(); inner++)
					{
						// Find label
						if (!MemoryBuff[iter].Destination.compare(MemoryBuff[inner].Label))
						{
							MemoryBuff[iter].Literal = MemoryBuff[inner].Address;
							break;
						}
						// Label not found
						// throw tantrum
					}
				}
			}
		}

		// If not an instruction
		if (MemoryBuff[iter].LabelDefinition)
		{
			// Try to convert to int (test for literal)
			try
			{
				MemoryBuff[iter].Literal = stoi(MemoryBuff[iter].Destination);
			}
			catch (const std::invalid_argument &ia)
			{
				std::cout << "Not a literal. Definition error" << ia.what() << std::endl;
				// throw tantrum
			}
		}
	}

	// Finally load the actual memory with binary
	for (size_t iter{}; iter < MemoryBuff.size(); iter++)
	{
		// Set whole word to literal
		MEMORY.push_back(MemoryBuff[iter].Literal);

		// If an instruction the four MSBs are the opcode
		if (MemoryBuff[iter].LabelledInstruction || MemoryBuff[iter].PlainInstruction)
		{
			MEMORY.back() = (MEMORY.back() & 4095) | (MemoryBuff[iter].Opcode << 12);
		}
	}

	return 0;
}

// Begin control functions for relay ALU
int ADD(ALU *RelayALU_ptr, int (ALU::*ClockTestALU_ptr)(), std::vector<uint16_t> &MEMORY, uint16_t* const address_ptr, uint16_t* const ACC_ptr)
{

	// Use lower half as inputs to relay ALU
	RelayALU_ptr->A = (*ACC_ptr) & LOWHALF;
	RelayALU_ptr->B = MEMORY[*address_ptr] & LOWHALF;
	// Use add instruction and execute
	RelayALU_ptr->Instruction = 0;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	// Set lower half to result
	(*ACC_ptr) &= HIGHHALF;
	(*ACC_ptr) |= RelayALU_ptr->Result;
	// Move higher half into ALU
	RelayALU_ptr->A = ((*ACC_ptr) & HIGHHALF) >> 8;
	RelayALU_ptr->B = (MEMORY[*address_ptr] & HIGHHALF) >> 8;
	// No carry in so use increment operation
	if (RelayALU_ptr->CarryOut)
	{
		RelayALU_ptr->Instruction = 1;
		(RelayALU_ptr->*ClockTestALU_ptr)();
		RelayALU_ptr->A = RelayALU_ptr->Result;
		RelayALU_ptr->Instruction = 0;
	}
	// Execute	
	(RelayALU_ptr->*ClockTestALU_ptr)();
	// Set higher half to result
	(*ACC_ptr) &= LOWHALF;
	(*ACC_ptr) |= uint16_t(RelayALU_ptr->Result) << 8;

	return 0;
}

int INC(ALU *RelayALU_ptr, int (ALU::*ClockTestALU_ptr)(), uint16_t* const ACC_ptr)
{
	// Lower half increment
	RelayALU_ptr->A = *ACC_ptr & LOWHALF;
	RelayALU_ptr->Instruction = 1;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= HIGHHALF;
	*ACC_ptr |= RelayALU_ptr->Result;

	// May need to carry in to upper half
	if (RelayALU_ptr->CarryOut)
	{
		RelayALU_ptr->A = (*ACC_ptr & HIGHHALF) >> 8;
		(RelayALU_ptr->*ClockTestALU_ptr)();
		*ACC_ptr &= LOWHALF;
		*ACC_ptr |= uint16_t(RelayALU_ptr->Result) << 8;
	}

	return 0;
}

int XOR(ALU *RelayALU_ptr, int (ALU::*ClockTestALU_ptr)(), std::vector<uint16_t> &MEMORY, uint16_t* const address_ptr, uint16_t* const ACC_ptr)
{
	// Xor relay instruction
	RelayALU_ptr->Instruction = 2;

	// Lower half
	RelayALU_ptr->A = *ACC_ptr & LOWHALF;
	RelayALU_ptr->B = MEMORY[*address_ptr] & LOWHALF;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= HIGHHALF;
	*ACC_ptr |= RelayALU_ptr->Result;

	// Upper half
	RelayALU_ptr->A = (*ACC_ptr & HIGHHALF) >> 8;
	RelayALU_ptr->B = (MEMORY[*address_ptr] & HIGHHALF) >> 8;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= LOWHALF;
	*ACC_ptr |= uint16_t(RelayALU_ptr->Result) << 8;

	return 0;
}

int AND(ALU *RelayALU_ptr, int (ALU::*ClockTestALU_ptr)(), std::vector<uint16_t> &MEMORY, uint16_t *address_ptr, uint16_t* const ACC_ptr)
{
	// And relay instruction
	RelayALU_ptr->Instruction = 3;

	// Lower half
	RelayALU_ptr->A = *ACC_ptr & LOWHALF;
	RelayALU_ptr->B = MEMORY[*address_ptr] & LOWHALF;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= HIGHHALF;
	*ACC_ptr |= RelayALU_ptr->Result;

	// Upper half
	RelayALU_ptr->A = (*ACC_ptr & HIGHHALF) >> 8;
	RelayALU_ptr->B = (MEMORY[*address_ptr] & HIGHHALF) >> 8;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= LOWHALF;
	*ACC_ptr |= uint16_t(RelayALU_ptr->Result) << 8;

	return 0;
}

int XNR(ALU *RelayALU_ptr, int (ALU::*ClockTestALU_ptr)(), std::vector<uint16_t> &MEMORY, uint16_t* const address_ptr, uint16_t* const ACC_ptr)
{
	// Xnor relay instruction
	RelayALU_ptr->Instruction = 4;

	// Lower half
	RelayALU_ptr->A = *ACC_ptr & LOWHALF;
	RelayALU_ptr->B = MEMORY[*address_ptr] & LOWHALF;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= HIGHHALF;
	*ACC_ptr |= RelayALU_ptr->Result;

	// Upper half
	RelayALU_ptr->A = (*ACC_ptr & HIGHHALF) >> 8;
	RelayALU_ptr->B = (MEMORY[*address_ptr] & HIGHHALF) >> 8;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= LOWHALF;
	*ACC_ptr |= uint16_t(RelayALU_ptr->Result) << 8;

	return 0;
}

int LOR(ALU *RelayALU_ptr, int (ALU::*ClockTestALU_ptr)(), std::vector<uint16_t> &MEMORY, uint16_t* const address_ptr, uint16_t* const ACC_ptr)
{
	// Or relay instruction
	RelayALU_ptr->Instruction = 5;

	// Lower half
	RelayALU_ptr->A = *ACC_ptr & LOWHALF;
	RelayALU_ptr->B = MEMORY[*address_ptr] & LOWHALF;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= HIGHHALF;
	*ACC_ptr |= RelayALU_ptr->Result;

	// Upper half
	RelayALU_ptr->A = (*ACC_ptr & HIGHHALF) >> 8;
	RelayALU_ptr->B = (MEMORY[*address_ptr] & HIGHHALF) >> 8;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= LOWHALF;
	*ACC_ptr |= uint16_t(RelayALU_ptr->Result) << 8;

	return 0;
}

int NOT(ALU *RelayALU_ptr, int (ALU::*ClockTestALU_ptr)(), uint16_t* const ACC_ptr)
{
	// Not relay instruction
	RelayALU_ptr->Instruction = 6;

	// Lower half negate
	RelayALU_ptr->A = *ACC_ptr & LOWHALF;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= HIGHHALF;
	*ACC_ptr |= RelayALU_ptr->Result;

	// Upper half negate
	RelayALU_ptr->A = (*ACC_ptr & HIGHHALF) >> 8;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= LOWHALF;
	*ACC_ptr |= uint16_t(RelayALU_ptr->Result) << 8;

	return 0;
}

int ROL(ALU *RelayALU_ptr, int (ALU::*ClockTestALU_ptr)(), uint16_t* const ACC_ptr)
{
	// Rotate relay instruction
	RelayALU_ptr->Instruction = 7;

	// Lower half rotate
	RelayALU_ptr->A = *ACC_ptr & LOWHALF;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= HIGHHALF;
	*ACC_ptr |= RelayALU_ptr->Result;

	// Upper half rotate
	RelayALU_ptr->A = (*ACC_ptr & HIGHHALF) >> 8;
	(RelayALU_ptr->*ClockTestALU_ptr)();
	*ACC_ptr &= LOWHALF;
	*ACC_ptr |= uint16_t(RelayALU_ptr->Result) << 8;

	// Need to swap two LSBs of 8-bit sub words
	const bool lowwordbit = bool(*ACC_ptr & 1);
	const bool highwordbit = bool(*ACC_ptr & (1 << 8));
	// Clear bits
	*ACC_ptr &= 65278;
	*ACC_ptr |= uint16_t(highwordbit);
	*ACC_ptr |= uint16_t(lowwordbit) << 8;

	return 0;
}

int SUB(ALU *RelayALU_ptr, int (ALU::*ClockTestALU_ptr)(), std::vector<uint16_t> &MEMORY, uint16_t* const address_ptr, uint16_t* const ACC_ptr)
{
	// No intrincsic sub operation so negate argument and add
	std::vector<uint16_t> buffer(1, MEMORY[*address_ptr]);
	uint16_t dummyaddress{};

	// Negate
	NOT(RelayALU_ptr, ClockTestALU_ptr, &(buffer[0]));

	// Increment
	INC(RelayALU_ptr, ClockTestALU_ptr, &(buffer[0]));

	// Add
	ADD(RelayALU_ptr, ClockTestALU_ptr, buffer, &dummyaddress, ACC_ptr);

	return 0;
}

int MUL(ALU *RelayALU_ptr, int (ALU::*ClockTestALU_ptr)(), std::vector<uint16_t> &MEMORY, uint16_t* const address_ptr, uint16_t* const ACC_ptr)
{
	// Load Buffer
	std::vector<uint16_t> buffer(1, *ACC_ptr);
	uint16_t dummyaddress{};

	// Clear Accumulator
	*ACC_ptr = 0;

	for (size_t iter{}; iter < 16; iter++)
	{
		// Multiply buffer by 2
		if (iter)
		{
			// Rotate left
			ROL(RelayALU_ptr, ClockTestALU_ptr, &(buffer[0]));
			// Remove lowest bit
			buffer[0] &= MU0MAXWORD - 1;
		}

		// If Multiplicand bit set, add buffer to ACC
		if ((MEMORY[*address_ptr] >> iter) & 1) ADD(RelayALU_ptr, ClockTestALU_ptr, buffer, &dummyaddress, ACC_ptr);
	}
	return 0;
}

int printstack(std::vector<uint16_t> &MEMORY, size_t START, size_t MAXLINES)
{
	// Start is origin to print around
	// MAXLINES is number of addresses either side to print

	int memlinesbefore = START;
	int memlinesafter = MEMORY.size() - START;

	if (memlinesafter < 0) return -1;

	if (memlinesbefore > MAXLINES) memlinesbefore = MAXLINES;
	if (memlinesafter > MAXLINES) memlinesafter = MAXLINES;

	std::cout << "       Stack (Hex)" << std::hex << std::endl;
	std::cout << "       ADDR [ADDR]" << std::hex << std::endl;

	for (size_t iter = START - memlinesbefore; iter < START; iter++)
	{
		std::cout << "       " << std::setfill('0') << std::setw(4) << iter << "  " << std::setfill('0') << std::setw(4) << MEMORY[iter] << std::endl;
	}

	std::cout << "-----> " << std::setfill('0') << std::setw(4) << START << "  " << std::setfill('0') << std::setw(4) << MEMORY[START] << std::endl;

	if (memlinesafter < 1)
	{
		std::cout << std::endl;
		return 0;
	}

	for (size_t iter = START + 1; iter < START + memlinesafter; iter++)
	{
		std::cout << "       " << std::setfill('0') << std::setw(4) << iter << "  " << std::setfill('0') << std::setw(4) << MEMORY[iter] << std::endl;
	}
	std::cout << std::endl;
	return 0;
}

int printstack(std::vector<uint16_t> &MEMORY, size_t START)
{
	size_t DefaultLines = 10;
	printstack(MEMORY, START, DefaultLines);
	return 0;
}

// From: http://stackoverflow.com/questions/865668/parse-command-line-arguments
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
	char ** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
	{
		return *itr;
	}
	return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
	return std::find(begin, end, option) != end;
}


int main(int argc, char* argv[])
{
	// Look for file name
	char * filename = getCmdOption(argv, argv + argc, "-f");
	std::string SourceFile;
	if (filename)
	{
		SourceFile = filename;
		std::cout << "Found text file: " << SourceFile << std::endl;
	}
	else
	{
		SourceFile = "input.txt";
		std::cout << "Text file not found/given, trying: " << SourceFile << std::endl;
	}
	std::cout << std::endl;

	// Look for clock speed
	char * clockchararray = getCmdOption(argv, argv + argc, "-c");
	double clockfrequency = 0;
	if (clockchararray)
	{
		clockfrequency = strtod(clockchararray, NULL);
		std::cout << "Input Clock Frequency: " << clockfrequency << "Hz" << std::endl;
	}
	else
	{
		std::cout << "Clock Frequency not found/given, assuming manual clock" << std::endl;
	}

	if (clockfrequency < 0)
	{
		std::cout << "Negative clock frequency, ignoring" << std::endl;
		clockfrequency = 0;
	}
	else if (clockfrequency > 10) std::cout << "Warning, Clock frequency may be too high" << std::endl;

	std::cout << std::endl;

	pause();

	// Processor memory 12-bit addressing
	std::vector<uint16_t> MEMORY;

	// Finished Variable
	bool finished = 0;

	// Registers
	// IR - Current Instruction
	// ACC - Accumulator
	// PC - Current Address
	uint16_t IR = 0, ACC = 0, PC = 0;
	uint16_t* const ACC_ptr = &ACC;

	// Instruction to execute	
	uint16_t address = 0, opcode = 0;
	uint16_t* const address_ptr = &address;

	// Set up interface
	ALU RelayALU;
	ALU* RelayALU_ptr = &RelayALU;
	int (ALU::*ClockTestALU_ptr)();
	ClockTestALU_ptr = &ALU::ClockTestALU;

	RelayALU.SetupInterface();
	RelayALU.ClockPeriod = 1000000 / clockfrequency;

	// Parse assembly text file and load memory
	if (LoadProgram(MEMORY, SourceFile))
	{
		std::cout << "Error loading program. Exiting..." << std::endl;
		return -1;
	}

	// Check memory usage
	std::cout << "Memory usage is " << MEMORY.size() * 2 << " out of 8192 bytes" << std::endl;
	if (MEMORY.size() >= MEMORY_LEN)
	{
		// Program too large
		std::cout << "Program size too large! Exiting..." << std::endl;
	}

	while ((!finished) && (PC < MEMORY.size()))
	{
		// Clear Console
		clearscreen();

		// Print Stack
		printstack(MEMORY, PC);
		std::cout << std::dec;

		// Get instruction
		IR = MEMORY[PC];

		// Increment program counter
		PC++;

		// Extract opcode & address
		opcode = IR >> 12;
		address = IR & (MEMORY_LEN - 1);

		// Get bitsets
		std::bitset<16> IR_bits(IR), OP_bits(opcode), ADDR_bits(address), PC_bits(PC), ACC_bits(ACC), TARGET_bits(MEMORY[address]);

		// Print Status
		std::cout << "       Bin              Dec   Hex" << std::endl;
		std::cout << "IR:    " << IR_bits << " " << std::setfill('0') << std::setw(5) << std::dec << IR << " " << std::setw(4) << std::hex << IR << std::endl;
		std::cout << "OP:    " << OP_bits << " " << std::setfill('0') << std::setw(5) << std::dec << opcode << " " << std::setw(4) << std::hex << opcode << std::endl;
		std::cout << "ADDR:  " << ADDR_bits << " " << std::setfill('0') << std::setw(5) << std::dec << address << " " << std::setw(4) << std::hex << address << std::endl;
		std::cout << "PC:    " << PC_bits << " " << std::setfill('0') << std::setw(5) << std::dec << PC << " " << std::setw(4) << std::hex << PC << std::endl;
		std::cout << "ACC:   " << ACC_bits << " " << std::setfill('0') << std::setw(5) << std::dec << ACC << " " << std::setw(4) << std::hex << ACC << std::endl;
		std::cout << std::endl;

		//Execute Instruction
		switch (opcode) {
		case 0:
			// LDA
			std::cout << "LDA" << std::endl;
			ACC = MEMORY[address];
			break;
		case 1:
			// STO
			std::cout << "STO" << std::endl;
			MEMORY[address] = ACC;
			break;
		case 2:
			// ADD
			std::cout << "ADD" << std::endl;
			std::cout << "[S]:   " << TARGET_bits << " " << std::setfill('0') << std::setw(5) << std::dec << MEMORY[address] << " " << std::setw(4) << std::hex << MEMORY[address] << std::endl;
			ADD(RelayALU_ptr, ClockTestALU_ptr, MEMORY, address_ptr, ACC_ptr);
			break;
		case 3:
			// SUB
			std::cout << "SUB" << std::endl;
			std::cout << "[S]:   " << TARGET_bits << " " << std::setfill('0') << std::setw(5) << std::dec << MEMORY[address] << " " << std::setw(4) << std::hex << MEMORY[address] << std::endl;
			SUB(RelayALU_ptr, ClockTestALU_ptr, MEMORY, address_ptr, ACC_ptr);
			break;
		case 4:
			// JMP
			std::cout << "JMP" << std::endl;
			PC = address;
			break;
		case 5:
			// JGE
			std::cout << "JGE" << std::endl;
			// Jump for positive integer i.e. MSB is 0
			if (!(ACC & (1 << 15)))
			{
				PC = address;
				std::cout << "PC Updated: " << std::dec << PC << std::endl;
			}
			else std::cout << "Not Jumping" << std::endl;
			break;
		case 6:
			// JNE
			// Jump for non zero accumulator
			std::cout << "JNE" << std::endl;
			if (ACC)
			{
				PC = address;
				std::cout << "PC Updated: " << std::dec << PC << std::endl;
			}
			else std::cout << "Not Jumping" << std::endl;
			break;
		case 7:
			// STP
			std::cout << "STP" << std::endl;
			finished = 1;
			break;
		case 8:
			// MUL
			std::cout << "MUL" << std::endl;
			std::cout << "[S]:   " << TARGET_bits << " " << std::setfill('0') << std::setw(5) << std::dec << MEMORY[address] << " " << std::setw(4) << std::hex << MEMORY[address] << std::endl;
			MUL(RelayALU_ptr, ClockTestALU_ptr, MEMORY, address_ptr, ACC_ptr);
			break;
		case 9:
			// INC
			std::cout << "INC" << std::endl;
			INC(RelayALU_ptr, ClockTestALU_ptr, ACC_ptr);
			break;
		case 10:
			// XOR
			std::cout << "XOR" << std::endl;
			std::cout << "[S]:   " << TARGET_bits << " " << std::setfill('0') << std::setw(5) << std::dec << MEMORY[address] << " " << std::setw(4) << std::hex << MEMORY[address] << std::endl;
			XOR(RelayALU_ptr, ClockTestALU_ptr, MEMORY, address_ptr, ACC_ptr);
			break;
		case 11:
			// AND
			std::cout << "AND" << std::endl;
			std::cout << "[S]:   " << TARGET_bits << " " << std::setfill('0') << std::setw(5) << std::dec << MEMORY[address] << " " << std::setw(4) << std::hex << MEMORY[address] << std::endl;
			AND(RelayALU_ptr, ClockTestALU_ptr, MEMORY, address_ptr, ACC_ptr);
			break;
		case 12:
			// XNR
			std::cout << "XNR" << std::endl;
			std::cout << "[S]:   " << TARGET_bits << " " << std::setfill('0') << std::setw(5) << std::dec << MEMORY[address] << " " << std::setw(4) << std::hex << MEMORY[address] << std::endl;
			XNR(RelayALU_ptr, ClockTestALU_ptr, MEMORY, address_ptr, ACC_ptr);
			break;
		case 13:
			// LOR
			std::cout << "LOR" << std::endl;
			std::cout << "[S]:   " << TARGET_bits << " " << std::setfill('0') << std::setw(5) << std::dec << MEMORY[address] << " " << std::setw(4) << std::hex << MEMORY[address] << std::endl;
			LOR(RelayALU_ptr, ClockTestALU_ptr, MEMORY, address_ptr, ACC_ptr);
			break;
		case 14:
			// NOT
			std::cout << "NOT" << std::endl;
			NOT(RelayALU_ptr, ClockTestALU_ptr, ACC_ptr);
			break;
		case 15:
			// ROL
			std::cout << "ROL" << std::endl;
			ROL(RelayALU_ptr, ClockTestALU_ptr, ACC_ptr);
			break;
		default:
			// Code
			break;
		}

		// Print updated ACC
		ACC_bits = ACC;
		std::cout << "ACC -> " << ACC_bits << " " << std::setfill('0') << std::setw(5) << std::dec << ACC << " " << std::setw(4) << std::hex << ACC << std::endl;
		std::cout << std::endl;

		if (clockfrequency == 0) pause();	

	}

	return 0;
}
