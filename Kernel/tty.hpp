#ifndef TTY_HPP
#define TTY_HPP

#include "Hardware/Drivers/qemu.hpp"
#include "Hardware/port.hpp"
#include "mutex.hpp"
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

#define MAX_ROWS 25
#define MAX_COLS 80
#define klog QemuSerial::active->qemu_debug

void kprintf(const char* format, ...);
void write_string(char* str);
void write_char(char c);
void clear_line(uint32_t y);
void clear_screen();
void set_color(uint8_t fg, uint8_t bg);
void update_cursor();
void set_cursor(bool enable);

#endif
