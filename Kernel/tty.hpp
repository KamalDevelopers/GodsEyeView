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

static char* datacolorblue = "\033[01;34m[GevOS]: ";
static char* datacoloroff = "\033[0m";
static bool serial_enabled = false;

static uint16_t* video_memory = (unsigned short*)0xb8000;
static int video_memory_index = 0;
static int new_line_index = 0;
static uint32_t default_color;
static uint32_t color;

/* Qemu serials */
void init_serial();
int transmit_empty();
void log_putc(char c);
void log_puts(char* str);
void klog(const char* format, ...);
void klog(int num);

void kprintf(const char* format, ...);
void write_string(char* str);
void write_char(int c);
void clear_screen(int fg = -1, int bg = -1);
void set_color(uint8_t fg, uint8_t bg);
void update_cursor();

#endif
