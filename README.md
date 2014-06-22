RelayProject
============
This repository contains files relating to a final year project, aimed at creating a relay-based arithmetic logic unit. Although obsolete, it is hoped that hardware is useful in demonstating basic computer science concepts, especially for use in teaching the new UK computer science curriculum. 

SOURCE Directory
----------------
* ALU_Control contains a C++ program which simulates the [MU0] [1] processor, but uses the relay ALU for intrinsic instructions like ADD, XOR, etc. Input is in the form of a human-readable assembly text file. It uses [WiringPi] [2] Raspberry Pi access library for the serial interfacing.
* Matrix Generation produces every possible network matrix for a given set of nodes. The problem size is n! hard (number of solutions is the corresponding [Bell number] [3])
* ProblemSize calculates the Bell number series
* SimulationTesting is a program that reads in the CSV output of the LTspice Relay ALU simulation, and checks the results against the expected operation for that combinatorial state.

  [1]: http://www.cs.man.ac.uk/~pjj/cs1001/arch/node1.html        "MU0"
  [2]: http://wiringpi.com/        "WiringPi"
  [3]: http://mathworld.wolfram.com/BellNumber.html        "Bell number"
