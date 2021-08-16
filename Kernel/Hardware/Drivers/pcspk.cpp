#include "pcspk.hpp"

void PCS::play_sound(uint32_t frequence)
{
    uint32_t div;
    uint8_t tmp;

    div = 1193180 / frequence;
    outb(0x43, 0xb6);
    outb(0x42, (uint8_t)(div));
    outb(0x42, (uint8_t)(div >> 8));

    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void PCS::nosound()
{
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void PCS::beep(uint32_t ms_time, uint32_t frequency)
{
    play_sound(frequency);
    usleep(ms_time);
    nosound();
}
