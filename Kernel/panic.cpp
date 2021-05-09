#include "panic.hpp"

struct stackframe {
    struct stackframe* ebp;
    uint32_t eip;
};

void dump_stack()
{
    struct stackframe* stk;
    stk = (stackframe*)__builtin_frame_address(0);

    for (unsigned int frame = 0; stk && frame < 4096; ++frame) {
        kprintf("0x%x ", stk->eip);
        stk = stk->ebp;
    }
}

[[noreturn]] void panic(char* error, const char* file, uint32_t line)
{
    kprintf("\33\x2\x4Kernel Panic! \33\x2\xF%s : %d \33\x2\x7\n* %s\n", file, line, error);
    IRQ::deactivate();
    kprintf("Stack Trace: ");
    dump_stack();

    while (1)
        ;
}
