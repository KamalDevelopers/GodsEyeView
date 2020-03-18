#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "LibGUI/font.hpp"

#include "GDT/gdt.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/interrupts.hpp"
#include "Hardware/Drivers/vga.hpp"
#include "Hardware/Drivers/mouse.hpp"

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
	for(constructor* i = &start_ctors; i != &end_ctors; i++)
		(*i)();
}

char* input(char brk, uint8_t color_index, KeyboardDriver *keyboard, Graphics *v)
{
	keyboard->ScreenOutput(2, color_index, v);
	char key;
	char* inp;

	while (key != brk)
	{
		key = keyboard->GetLastKey();
	}
	keyboard->GetKeys(inp);
	return inp;
}

extern "C" void kernelMain(void* multiboot_structure, unsigned int magicnumber)
{
	Graphics vga;
	vga.Init(320, 200, 256, 0x0);
	while(1);
}