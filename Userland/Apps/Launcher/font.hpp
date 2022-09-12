#ifndef FONT_HPP
#define FONT_HPP

#include <LibC/types.h>
#include <LibDisplay/canvas.hpp>

typedef struct psf_font {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
} psf_font_t;

void display_character(canvas_t* canvas, psf_font_t* font, uint16_t c, int start_x, int start_y, uint32_t fg, uint32_t bg);

#endif
