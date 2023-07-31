#ifndef VIRTUAL_HPP
#define VIRTUAL_HPP

#include "../port.hpp"
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <stdarg.h>

/* TODO: VMware Serial interface class */

#define IS_RUNNING_IN_QEMU true

class QemuSerial {
private:
    bool support_qemu_serial = false;

public:
    QemuSerial();
    ~QemuSerial();

    static QemuSerial* active;

    bool is_supported() { return support_qemu_serial; }
    void wait_transmit_empty();
    void putc(char character);
    void puts(char* text);
};

#endif
