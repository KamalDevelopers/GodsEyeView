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
	Graphics vga;
	vga.Init(640, 480, 16, 0x2);
	

	for (int i = 0; i < 50; i++){
		vga.PutPixel(i, 1, 0xFC, 0xFC, 0xFC);
	}
	//clear();
	while(1);
}