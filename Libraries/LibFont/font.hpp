#ifndef LIB_FONT_HPP
#define LIB_FONT_HPP

#include <LibC/stat.h>
#include <LibC/stdlib.h>
#include <LibC/types.h>
#include <LibC/unistd.h>
#include <LibDisplay/canvas.hpp>

typedef struct font_char_header {
    int x_offset;
    int y_offset;
    uint32_t ptr;
} __attribute__((packed)) font_char_header_t;

typedef struct font_header {
    uint8_t width;
    uint8_t height;
    font_char_header_t chars[127];
} __attribute__((packed)) font_header_t;

typedef struct font {
    font_header_t* font_header;
    uint8_t* font_buffer;
} font_t;

uint32_t font_display_character_with_bg(font_t* use_font, canvas_t* canvas, char c, uint32_t pos_x,
    uint32_t pos_y, uint32_t color, uint32_t bg, bool use_bg);
uint32_t font_display_character(font_t* use_font, canvas_t* canvas, char c, uint32_t pos_x,
    uint32_t pos_y, uint32_t color);
font_t* font_load(const char* pathname);
void font_unload(font_t* font);

#endif
