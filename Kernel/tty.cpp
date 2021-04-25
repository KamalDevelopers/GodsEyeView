#include "tty.hpp"

void kprintf(const char* format, ...)
{
    va_list arg;

    puts_hook(write_string);
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
    puts_hook(0);
}

void init_serial()
{
    outb(COMPORT + 1, 0x00);
    outb(COMPORT + 3, 0x80);
    outb(COMPORT + 0, 0x03);
    outb(COMPORT + 1, 0x00);
    outb(COMPORT + 3, 0x03);
    outb(COMPORT + 2, 0xC7);
    outb(COMPORT + 4, 0x0B);
}

int transmit_empty()
{
    return inb(COMPORT + 5) & 0x20;
}

void log_putc(char c)
{
    while (transmit_empty() == 0)
        ;
    outb(COMPORT, c);
}

void klog(char* str)
{
    for (int i = 0; i < strlen(datacolorblue); i++)
        log_putc(datacolorblue[i]); //color on
    for (int i = 0; str[i] != '\0'; i++) {
        log_putc(str[i]);
    }
    log_putc('\n');
    for (int i = 0; i < strlen(datacoloroff); i++)
        log_putc(datacoloroff[i]); //color off
}

void klog(int num)
{
    char str[20];
    itoa(num, str);

    for (int i = 0; i < strlen(datacolorblue); i++)
        log_putc(datacolorblue[i]); //color on
    for (int i = 0; str[i] != '\0'; i++) {
        log_putc(str[i]);
    }
    log_putc('\n');
    for (int i = 0; i < strlen(datacoloroff); i++)
        log_putc(datacoloroff[i]); //color off
}

void write_string(char* str)
{
    if (strcmp(str, "\33[H\33[2J") == 0) {
        clear_screen();
        return;
    }
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\b') {
            if (VideoMemoryIndex <= 0)
                continue;
            VideoMemoryIndex--;
            write_char(' ');
            VideoMemoryIndex--;
            continue;
        }

        write_char(str[i]);
    }
}

void write_char(int c)
{
    if (c == 10) {
        NewLineIndex++;
        VideoMemoryIndex = 0;
    } else {
        VideoMemory[80 * NewLineIndex + VideoMemoryIndex] = (VideoMemory[VideoMemoryIndex] & 0xff00) | c;
        VideoMemoryIndex++;
    }

    if (NewLineIndex >= MAX_ROWS) {
        for (int y = 0; y < MAX_ROWS; y++) {
            for (int x = 0; x < MAX_COLS; x++) {
                VideoMemory[80 * y + x] = VideoMemory[80 * (y + 1) + x];
            }
        }

        VideoMemoryIndex = 0;
        NewLineIndex = MAX_ROWS - 1;
    }

    if (VideoMemoryIndex >= MAX_COLS) {
        VideoMemoryIndex = 0;
        NewLineIndex++;
    }
}

void clear_screen()
{
    VideoMemoryIndex = 0;
    NewLineIndex = 0;
    for (int i = 0; i < 2200; i++) {
        VideoMemory[i] = (VideoMemory[i] & 0xff00) | ' ';
    }
}
