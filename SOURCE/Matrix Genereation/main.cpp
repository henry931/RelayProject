#include <stdint.h>
#include <vector>
#include <math.h> 
#include <iostream>

// Vector to store locations of zeros
struct ZeroVector { uint64_t i, j; };

// Node Merge Function
void MergeNodes(std::vector<uint64_t>* Matrix, ZeroVector MergeVector, uint64_t Height)
{
	// Result of all joined nodes (as rows)
	uint64_t Result = 0;
	uint64_t PrevResult = 0;

	// Find result (merged rows of nodes to be merged)
	Result = (*Matrix)[MergeVector.i - 1] | (*Matrix)[MergeVector.j - 1];

	// Keep going until we iterate through all rows with no change
	while (Result != PrevResult)
	{
		PrevResult = Result;

		// For each row
		for (uint64_t row = 0; row < Height; row++)
		{
			// Check if rows (nodes) are connected
			if ((*Matrix)[row] & Result)
			{
				// Merge these rows
				Result |= (*Matrix)[row];
				(*Matrix)[row] = Result;
			}
		}
	}
}

// From: http://stackoverflow.com/questions/15301885/calculate-value-of-n-choose-k
uint64_t NChooseK(uint64_t n, uint64_t k){

	if (k == 0) return 1;

	return (n * NChooseK(n - 1, k - 1)) / k;
}


int main()
{
	// Scope of problem size calculation
	uint64_t EndTarget = 24;
	std::vector<uint64_t> Result(EndTarget);

	Result[0] = 1;

	// Iterate
	for (uint64_t i = 0; i < EndTarget - 1; i++)
	{
		uint64_t Summation = 0;

		for (uint64_t j = 0; j <= i; j++) Summation += Result[j] * NChooseK(i, j);

		Result[i + 1] = Result[i] + Summation;
	}

	// Number of total nodes
	// Max 64 for this program
	uint64_t Nodes = 10;

	// Iteration Number
	// uint64_t Iteration = 0;

	// Index numbers for each iteration
	std::vector<uint64_t> IterStartIndex(Nodes), IterEndIndex(Nodes);

	// Total Number of possible combinations
	//uint64_t Combinations = std::pow(2, Nodes*(Nodes-1)/2);

	// Form the net matrix - each vector element represents a row
	// Outer vector is number of total matrices
	std::vector<std::vector<uint64_t> > Matrices(1, std::vector<uint64_t>(Nodes));
	// Number of results known, preallocate memory
	Matrices.reserve(Result[Nodes]);
	// Get address of working matrix
	uint64_t* CurrentMatrix = &Matrices[0][0];

	// Initialise leading diagonal - Completely disconnected network
	// Origin effectively at top right of matrix
	// i.e. move left for increasing colums and down for rows
	for (uint64_t iter = 0; iter < Nodes; iter++)
		CurrentMatrix[iter] |= uint64_t(std::pow(2, iter));
	IterStartIndex[0] = IterEndIndex[0] = 0;

	for (uint64_t iter = 1; iter < Nodes; iter++)
	{
		// Update start index for this iteration
		IterStartIndex[iter] = IterEndIndex[iter - 1] + 1;

		// Set end index to previous iteration to
		// represent no matrices
		IterEndIndex[iter] = IterEndIndex[iter - 1];

		for (uint64_t bmat = IterStartIndex[iter - 1]; bmat <= IterEndIndex[iter - 1]; bmat++)
		{
			// Create counter to count number of found
			// matrices for this base matrix
			//uint64_t MatricesFound = 0;

			// Get address of working matrix
			uint64_t* BaseMatrix = &Matrices[bmat][0];

			// Create buffer vector containing all possible new connections
			std::vector<ZeroVector> Zeros;
			uint64_t NumZeroVectors = 0;

			// Find all locations of zeroes - i.e. disconnected nodes
			for (uint64_t i = 1; i < Nodes; i++)
			{
				uint64_t* CurrentRow = &BaseMatrix[i - 1];

				// Only search above leading diagonal as symmetric matrix
				// i.e. if node 1 is connected to 3
				// ...node 3 is therefore connected to 1
				for (uint64_t j = i + 1; j <= Nodes; j++)
				{
					if (!((*CurrentRow >> j - 1) & 1))
					{
						ZeroVector FoundVector;
						FoundVector.i = i;
						FoundVector.j = j;
						Zeros.push_back(FoundVector);
						NumZeroVectors++;
					}
				}
			}

			// For each zero...
			for (uint64_t ZeroIteration = 0; ZeroIteration < NumZeroVectors; ZeroIteration++)
			{
				// Create buffer matrix
				std::vector<uint64_t> BufferMatrix(Nodes);
				BufferMatrix = Matrices[bmat];

				// Perform merge - connect nodes
				MergeNodes(&BufferMatrix, Zeros[ZeroIteration], Nodes);

				// Check they don't already exist in this iteration space
				unsigned MatrixUnique = 1;
				for (uint64_t i = IterStartIndex[iter]; i <= IterEndIndex[iter]; i++)
				{
					/*unsigned MatrixNotUnique = 1;
					for (uint64_t rowcheck = 0; rowcheck < Nodes; rowcheck++)
					{
						if (BufferMatrix[rowcheck] != Matrices[i][rowcheck])
						{
							MatrixNotUnique = 0;
							break;
						}
					}
					if (MatrixNotUnique == 1)
					{
						MatrixUnique = 0;
						break;
					}*/
						if (BufferMatrix == Matrices[i])
					{
					MatrixUnique = 0;
					break;
					}
				}

				if (MatrixUnique == 1)
				{
					// -> Increment End Index
					IterEndIndex[iter]++;
					// -> Copy to this iteration space...
					Matrices.push_back(BufferMatrix);
					// check iof finished
					if (Matrices.size() == Result[Nodes]) break;
				}

				// Update end index
				//IterEndIndex[iter] += MatricesFound;

			}
		}
	}
	std::cout << "Matrix Size: " << Matrices.size() << std::endl;

	return 0;
}