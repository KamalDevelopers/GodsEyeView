#include "stdio.hpp"
#include "stdlib.hpp"
#include "../libraries/LibGUI/gui.hpp"
#include "../libraries/LibGUI/font.hpp"

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
	GlobalDescriptorTable gdt;
	InterruptManager interrupts(0x20, &gdt);
	GUI::Desktop desktop(320, 200, &vga);

	MouseDriver mouse(&interrupts, &vga, &desktop);
	interrupts.Activate();

	GUI::Window window(44, 11, 200, 60, 0x1);
	GUI::Window *win = &window;

	GUI::Window windowTwo(80, 100, 160, 30, 0x3);
	GUI::Window *winT = &windowTwo;

	GUI::Label label(0, 50, 1, 10, 0x4, 0x2, "First Window");
	GUI::Label *lbl = &label;

	GUI::Label labelTwo(50, 0, 1, 10, 0x4, 0x2, "Second Window");
	GUI::Label *lblT = &labelTwo;

	win->AddWidget(1, lbl);
	winT->AddWidget(1, lblT);

	desktop.AddWin(2, win, winT);
	desktop.Draw();

	while(1);
}