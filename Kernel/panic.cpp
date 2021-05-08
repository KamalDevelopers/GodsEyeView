#include "panic.hpp"

[[noreturn]] void panic(char* error)
{
    set_color(0x4, 0x0);
    kprintf("Kernel Panic!\n");
    set_color(0x7, 0x0);
    kprintf("* %s", error);
    while (1)
        ;
}
