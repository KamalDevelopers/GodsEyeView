#include "tty.hpp"

MUTEX(console);

uint16_t* video_memory = (unsigned short*)0xb8000;
bool serial_enabled = false;
bool cursor_enabled = true;
int video_memory_index = 0;
int new_line_index = 0;
uint32_t default_color = 0;
uint32_t color = 0;

void kprintf(const char* format, ...)
{
    va_list arg;

    TM->deactivate();
    puts_hook(write_string);
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
    puts_hook(0);
    TM->activate();
}

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg)
{
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

static int8_t esc_flag = 0;
void write_string(char* str)
{
    Mutex::lock(console);
    for (uint32_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\b' && !esc_flag) {
            if (video_memory_index <= 0)
                continue;
            video_memory_index--;
            write_char(' ');
            video_memory_index--;
            update_cursor();
            continue;
        }

        write_char(str[i]);
    }
    Mutex::unlock(console);
}

void write_char(char c)
{
    if (c == '\33' && !esc_flag) {
        esc_flag = 1;
        return;
    }

    if (esc_flag == 1) {
        esc_flag = 0;
        switch (c) {
        case 1:
            clear_screen();
            return;
        case 2:
            esc_flag = 2;
            return;
        case 3:
            color = default_color;
            return;
        case 4:
            esc_flag = 3;
            return;
        case 5:
            esc_flag = 4;
            return;
        }
    }

    if (esc_flag == 2) {
        esc_flag = 0;
        set_color(c, 0);
        return;
    }

    if (esc_flag == 3) {
        esc_flag = 0;
        video_memory_index = c - 1;
        return;
    }

    if (esc_flag == 4) {
        esc_flag = 0;
        new_line_index = c - 1;
        return;
    }

    if (c == 10) {
        new_line_index++;
        video_memory_index = 0;
        clear_line(new_line_index);
    } else {
        video_memory[80 * new_line_index + video_memory_index] = vga_entry(c, color);
        video_memory_index++;
    }

    if (new_line_index >= MAX_ROWS) {
        for (uint32_t y = 0; y < MAX_ROWS; y++) {
            for (uint32_t x = 0; x < MAX_COLS; x++) {
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

    update_cursor();
}

void clear_line(uint32_t y)
{
    for (uint32_t x = 0; x < MAX_COLS; x++) {
        video_memory[80 * new_line_index + x] = (video_memory[80 * new_line_index + x] & 0xff00) | ' ';
        video_memory[80 * new_line_index + x] = vga_entry(' ', color);
    }
}

void clear_screen()
{
    video_memory_index = 0;
    new_line_index = 0;
    for (uint32_t i = 0; i < MAX_COLS * MAX_ROWS; i++) {
        video_memory[i] = (video_memory[i] & 0xff00) | ' ';
        video_memory[i] = vga_entry(' ', color);
    }
}

void set_color(uint8_t fg, uint8_t bg)
{
    color = vga_entry_color(fg, bg);
    if (default_color == 0)
        default_color = color;
}

void update_cursor()
{
    if (!cursor_enabled)
        return;

    uint32_t pos = new_line_index * 80 + video_memory_index;
    outb(0x3D4, 15);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 14);
    outb(0x3D5, (uint8_t)(pos >> 8) & 0xFF);
}

void set_cursor(bool enable)
{
    if (!enable) {
        cursor_enabled = false;
        outb(0x3D4, 0x0A);
        outb(0x3D5, 0x20);
        return;
    }

    cursor_enabled = true;
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 14);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
    update_cursor();
}
