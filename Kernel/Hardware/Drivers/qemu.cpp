#include "qemu.hpp"

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

void write_qemu_serial_hook(char* text)
{
    QemuSerial::active->puts(text);
}

void QemuSerial::qemu_debug(const char* format, ...)
{
    const char* klog_prefix = "\033[01;34m[GevOS]: ";
    const char* klog_suffix = "\033[0m\n";

    puts((char*)klog_prefix);

    va_list arg;
    puts_hook(write_qemu_serial_hook);
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
    puts_hook(0);

    puts((char*)klog_suffix);
}
