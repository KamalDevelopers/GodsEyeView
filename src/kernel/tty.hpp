#ifndef TTY_HPP
#define TTY_HPP

#define MAX_ROWS 25
#define MAX_COLS 80

#include "itoa.hpp"
#include "string.hpp"

static uint16_t* VideoMemory = (unsigned short*)0xb8000;
static int VideoMemoryIndex = 0;
static int NewLineIndex = 0;

extern void write_string(char* str);
extern void write_char(int c);
extern void clear_screen();
#endif
