#ifndef QEMU_HPP
#define QEMU_HPP

#include "../port.hpp"
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <stdarg.h>

class QemuSerial {
private:
    bool support_qemu_serial = true;

public:
    QemuSerial();
    ~QemuSerial();

    static QemuSerial* active;

    void wait_transmit_empty();
    void putc(char character);
    void puts(char* text);
    void qemu_debug(const char* format, ...);
};

#endif
