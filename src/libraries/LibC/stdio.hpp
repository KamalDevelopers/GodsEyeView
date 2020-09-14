#ifndef STDIO_HPP
#define STDIO_HPP

#define COMPORT 0x3f8
#define MAX_ROWS 25
#define MAX_COLS 80

#include "itoa.hpp"
#include "string.hpp"
#include <stdarg.h>

static char* datacolorblue = "\033[01;34m[GevOS]: ";
static char* datacoloroff = "\033[0m";

extern void puts(char* str);
extern void puti(int num);
extern void putc(int c);
extern void putx(int c);
extern void vprintf(const char* format, va_list v);
extern void printf(const char* format, ...);
extern void clear();
extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);
extern void sleep(uint32_t timer_count);
extern void usleep(uint32_t ms);
extern void PCS_play_sound(uint32_t nFrequence);
extern void PCS_nosound();
extern void beep(int time, int frequency);
static void newline()
{
    putc(10);
}

/*Serials*/
extern void init_serial();
extern int transmit_empty();
extern void log_putc(char c);
extern void klog(char* str);
extern void klog(int num);
#endif
