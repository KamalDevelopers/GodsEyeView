#include "tty.hpp"

void kprintf(char* str)
{
    write_string(str);
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
