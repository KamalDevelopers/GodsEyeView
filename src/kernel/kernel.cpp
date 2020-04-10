#include "../libraries/LibGUI/font.hpp"
#include "../libraries/LibGUI/gui.hpp"
#include "stdio.hpp"
#include "stdlib.hpp"

#include "GDT/gdt.hpp"
#include "Hardware/Drivers/cmos.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/mouse.hpp"
#include "Hardware/Drivers/vga.hpp"
#include "Hardware/interrupts.hpp"

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(void* multiboot_structure, unsigned int magicnumber)
{
    Graphics vga;
    vga.Init(640, 480, 16, 0);

    while (1)
        ;
}