#ifndef TTY_HPP
#define TTY_HPP

#define COMPORT 0x3f8
#define MAX_ROWS 25
#define MAX_COLS 80

#include "Hardware/port.hpp"
#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "LibC/string.hpp"
#include <stdarg.h>

static char* datacolorblue = "\033[01;34m[GevOS]: ";
static char* datacoloroff = "\033[0m";

/* Serials */
extern void init_serial();
extern int transmit_empty();
extern void log_putc(char c);
extern void klog(char* str);
extern void klog(int num);

static uint16_t* VideoMemory = (unsigned short*)0xb8000;
static int VideoMemoryIndex = 0;
static int NewLineIndex = 0;

void kprintf(const char* format, ...);
void write_string(char* str);
void write_char(int c);
void clear_screen();

#endif
