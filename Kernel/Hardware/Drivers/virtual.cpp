#include "virtual.hpp"

QemuSerial* QemuSerial::active = 0;
QemuSerial::QemuSerial()
{
    active = this;
    if (!support_qemu_serial)
        return;

    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

QemuSerial::~QemuSerial()
{
}

void QemuSerial::wait_transmit_empty()
{
    if (!support_qemu_serial)
        return;
    while ((inb(0x3F8 + 5) & 0x20) == 0)
        ;
}

void QemuSerial::putc(char character)
{
    if (!support_qemu_serial)
        return;

    wait_transmit_empty();
    outb(0x3F8, character);
}

void QemuSerial::puts(char* text)
{
    for (uint32_t i = 0; text[i] != 0; i++)
        putc(text[i]);
}
