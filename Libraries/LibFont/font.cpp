#include "font.hpp"

uint32_t font_display_character(font_t* use_font, canvas_t* canvas, char c, uint32_t pos_x,
    uint32_t pos_y, uint32_t color)
{
    return font_display_character_with_bg(use_font, canvas, c, pos_x, pos_y, color, 0, false);
}

uint32_t font_display_character_with_bg(font_t* use_font, canvas_t* canvas, char c, uint32_t pos_x,
    uint32_t pos_y, uint32_t color, uint32_t bg, bool use_bg)
{
    if (c >= 127)
        return pos_x;
    uint32_t offset = use_font->font_header->chars[c].ptr;
    pos_y += use_font->font_header->chars[c].y_offset;
    pos_x += use_font->font_header->chars[c].x_offset;
    uint32_t* ptr = canvas->framebuffer + pos_y * canvas->width + pos_x;
    uint8_t* font = use_font->font_buffer + offset;

    uint32_t cache_alpha = 0;
    uint32_t cache_pixel = 0;
    uint32_t cache_color = 0;
    uint32_t cache_pixel_value = 0;

    for (uint32_t y = 0; y < use_font->font_header->height; y++) {
        uint32_t* dptr = ptr;
        for (uint32_t x = 0; x < use_font->font_header->width; x++) {
            uint8_t alpha = *font;
            uint32_t orig = (use_bg) ? bg : *dptr;
            if (c == ' ') {
                *dptr = bg;
                dptr++;
                font++;
                continue;
            }

            if (cache_color == color && cache_pixel == orig && cache_alpha == alpha) {
                *dptr = cache_pixel_value;
            } else {
                uint32_t blend_orig = orig;
                if (alpha > 200)
                    blend_orig = ((orig >> 16) & 0x000000FF) + ((orig >> 8) & 0x000000FF) + ((orig >> 0) & 0x000000FF);
                *dptr = pixel_alpha_blend(color, blend_orig, alpha);
                cache_color = color;
                cache_pixel = orig;
                cache_alpha = alpha;
                cache_pixel_value = *dptr;
            }

            dptr++;
            font++;
        }
        ptr += canvas->width;
    }

    return pos_x + use_font->font_header->width + 2;
}

void font_unload(font_t* font)
{
    if (!font)
        return;
    font->font_buffer = 0;
    font->font_header = 0;
    free(font->font_buffer);
    free(font);
}

font_t* font_load(const char* name)
{
    font_t* font = (font_t*)malloc(sizeof(font_t));
    memset(font, 0, sizeof(font_t));
    struct stat statbuffer;
    int file_descriptor = open("bitmaps/font.tftf", O_RDONLY);
    fstat(file_descriptor, &statbuffer);
    font->font_buffer = (uint8_t*)malloc(sizeof(char) * statbuffer.st_size);
    memset(font->font_buffer, 0, sizeof(char) * statbuffer.st_size);
    read(file_descriptor, font->font_buffer, statbuffer.st_size);
    close(file_descriptor);
    font->font_header = (font_header*)font->font_buffer;
    return font;
}
