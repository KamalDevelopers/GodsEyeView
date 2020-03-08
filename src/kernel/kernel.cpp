#include "printf.hpp"

extern "C" void kernelMain(void* multiboot_structure, unsigned int magicnumber)
{
	clear();
	printf("%s", "GevOS");
	while(1);
}