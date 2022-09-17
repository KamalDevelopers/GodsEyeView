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

uint32_t font_display_character(canvas_t* canvas, char c, uint32_t pos_x,
    uint32_t pos_y, uint32_t color, uint32_t bg = 0, bool use_bg = false);
font_header_t* current_font_header();
void font_load(const char* name);
void font_unload();

#endif
