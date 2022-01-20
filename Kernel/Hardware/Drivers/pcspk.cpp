#include "pcspk.hpp"

void PCS::beep_start(uint32_t frequency)
{
    uint32_t div;
    uint8_t tmp;

    div = 1193180 / frequency;
    outb(0x43, 0xb6);
    outb(0x42, (uint8_t)(div));
    outb(0x42, (uint8_t)(div >> 8));

    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void PCS::beep_stop()
{
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}
