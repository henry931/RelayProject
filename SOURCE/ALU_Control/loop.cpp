// Combinatorial test loop - may take a while
for (size_t iin{}; iin < 8; iin++)
{
	RelayALU.Instruction = iin;

	for (size_t ain{}; ain < 256; ain++)
	{
		RelayALU.A = ain;

		for (size_t bin{}; bin < 256; bin++)
		{
			RelayALU.B = bin;

			RelayALU.ClockALU();
		}
	}
}