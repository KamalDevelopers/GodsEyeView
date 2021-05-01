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

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg)
{
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

void write_string(char* str)
{
    if (strcmp(str, "\33[H\33[2J") == 0) {
        clear_screen();
        return;
    }
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\b') {
            if (video_memory_index <= 0)
                continue;
            video_memory_index--;
            write_char(' ');
            video_memory_index--;
            continue;
        }

        write_char(str[i]);
    }
}

void write_char(int c)
{
    if (c == 10) {
        new_line_index++;
        video_memory_index = 0;
    } else {
        //video_memory[80 * new_line_index + video_memory_index] = (video_memory[video_memory_index] & 0xff00) | c;
        video_memory[80 * new_line_index + video_memory_index] = vga_entry(c, color);
        video_memory_index++;
    }

    if (new_line_index >= MAX_ROWS) {
        for (int y = 0; y < MAX_ROWS; y++) {
            for (int x = 0; x < MAX_COLS; x++) {
                video_memory[80 * y + x] = video_memory[80 * (y + 1) + x];
            }
        }

        video_memory_index = 0;
        new_line_index = MAX_ROWS - 1;
    }

    if (video_memory_index >= MAX_COLS) {
        video_memory_index = 0;
        new_line_index++;
    }
}

void clear_screen(int fg, int bg)
{
    if ((fg != -1) && (bg != -1))
        set_color(fg, bg);

    video_memory_index = 0;
    new_line_index = 0;
    for (int i = 0; i < 2200; i++) {
        video_memory[i] = (video_memory[i] & 0xff00) | ' ';
        video_memory[i] = vga_entry(' ', color);
    }
}

void set_color(uint8_t fg, uint8_t bg)
{
    color = vga_entry_color(fg, bg);
}
