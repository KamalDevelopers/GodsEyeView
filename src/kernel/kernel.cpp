#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "LibGUI/font.hpp"
#include "GDT/gdt.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/interrupts.hpp"
#include "Hardware/Drivers/vga.hpp"

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
	for(constructor* i = &start_ctors; i != &end_ctors; i++)
		(*i)();
}

extern "C" void kernelMain(void* multiboot_structure, unsigned int magicnumber)
{
	clear();
	printf("%s", "GevOS");
	while(1);
}