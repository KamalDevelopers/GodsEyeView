#ifndef TTY_HPP
#define TTY_HPP

#define COMPORT 0x3f8
#define MAX_ROWS 25
#define MAX_COLS 80

#include "Hardware/port.hpp"
#include "mutex.hpp"
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <stdarg.h>

/* Qemu serials */
void init_serial();
int transmit_empty();
void log_putc(char c);
void log_puts(char* str);
void klog(const char* format, ...);
void klog(int num);

void kprintf(const char* format, ...);
void write_string(char* str);
void write_char(char c);
void clear_line(uint32_t y);
void clear_screen();
void set_color(uint8_t fg, uint8_t bg);
void update_cursor();
void set_cursor(bool enable);

#endif
