#include "pcs.hpp"

void PCS::beep_on(uint32_t frequency)
{
    outb(0x43, 0xB6);
    uint16_t timer_reload = BASE_FREQUENCY / frequency;

    outb(0x42, LSB(timer_reload));
    outb(0x42, MSB(timer_reload));
    outb(0x61, inb(0x61) | 3);
}

void PCS::beep_off()
{
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}
