#include "font.hpp"

void display_character(canvas_t* canvas, psf_font_t* font, uint16_t c, int start_x, int start_y, uint32_t fg, uint32_t bg)
{
    int bytesperline = (font->width + 7) / 8;
    int scanline = canvas->width * sizeof(int32_t);
    uint8_t* glyph = (uint8_t*)font + font->headersize + (c > 0 && c < font->numglyph ? c : 0) * font->bytesperglyph;
    int offs = (start_y * scanline) + (start_x * sizeof(uint32_t));
    const uint8_t masks[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
    int line = 0;

    for (int y = 0; y < font->height; y++) {
        line = offs;

        for (int x = 0; x < font->width; x++) {
            *((uint32_t*)(((uint8_t*)canvas->framebuffer) + line)) = *((uint32_t*)glyph) & masks[x] ? fg : bg;
            line += sizeof(uint32_t);
        }

        glyph += bytesperline;
        offs += scanline;
    }
}
