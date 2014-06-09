#include <stdint.h>
#include <vector>
#include <iostream>
#include <bitset>

// From: http://stackoverflow.com/questions/15301885/calculate-value-of-n-choose-k
uint64_t NChooseK(uint64_t n, uint64_t k){

	if (k == 0) return 1;

	return (n * NChooseK(n - 1, k - 1)) / k;
}

int main()
{
	uint64_t EndTarget = 24;

	std::vector<uint64_t> Result(EndTarget);

	Result[0] = 1;

	for (uint64_t i = 0; i < EndTarget - 1; i++)
	{
		uint64_t Summation = 0;

		for (uint64_t j = 0; j <= i; j++) Summation += Result[j] * NChooseK(i, j);

		Result[i + 1] = Result[i] + Summation;
	}

	for (size_t i = 0; i < EndTarget; i++)
	{
		std::cout << Result[i] << std::endl;
	}

	std::cout << std::endl;

	for (size_t i = 0; i < EndTarget; i++)
	{
		std::bitset<64> x(Result[i]);

		std::cout << x << std::endl;
	}

	std::cout << std::endl;

	for (size_t i = 0; i < EndTarget; i++)
	{
		std::cout << float(Result[i]) << std::endl;
	}

}