#ifndef TTY_HPP
#define TTY_HPP

#define MAX_ROWS 25
#define MAX_COLS 80

#include "LibC/stdlib.hpp"
#include "LibC/string.hpp"

static uint16_t* VideoMemory = (unsigned short*)0xb8000;
static int VideoMemoryIndex = 0;
static int NewLineIndex = 0;

void kprintf(char* str);
void write_string(char* str);
void write_char(int c);
void clear_screen();
#endif
