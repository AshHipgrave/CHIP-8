#include "Emulator.h"

int main(int argc, char* args[])
{
	Emulator emulator;

	if (!emulator.Initialise())
	{
		std::cout << "Initialisation failed!" << std::endl;
		return 1;
	}

	emulator.Run();

	return 0;
}