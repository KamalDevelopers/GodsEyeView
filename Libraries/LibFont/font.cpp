#include "font.hpp"

static font_header_t* font_header = 0;
static uint8_t* font_buffer = 0;

uint32_t font_display_character(canvas_t* canvas, char c, uint32_t pos_x,
    uint32_t pos_y, uint32_t color, uint32_t bg, bool use_bg)
{
    uint32_t offset = font_header->chars[c].ptr;
    pos_y += font_header->chars[c].y_offset;
    pos_x += font_header->chars[c].x_offset;
    uint32_t* ptr = canvas->framebuffer + pos_y * canvas->width + pos_x;
    uint8_t* font = font_buffer + offset;

    uint32_t cache_alpha = 0;
    uint32_t cache_pixel = 0;
    uint32_t cache_color = 0;
    uint32_t cache_pixel_value = 0;

    for (uint32_t y = 0; y < font_header->height; y++) {
        uint32_t* dptr = ptr;
        for (uint32_t x = 0; x < font_header->width; x++) {
            uint8_t alpha = *font;
            uint32_t orig = (use_bg) ? bg : *dptr;

            if (cache_color == color && cache_pixel == orig && cache_alpha == alpha) {
                *dptr = cache_pixel_value;
            } else {
                *dptr = pixel_alpha_blend(color, orig, alpha);
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

    return pos_x + font_header->width + 2;
}

void font_unload()
{
    if (!font_buffer)
        return;
    free(font_buffer);
    font_buffer = 0;
    font_header = 0;
}

void font_load(const char* name)
{
    if (font_buffer)
        font_unload();

    struct stat statbuffer;
    int file_descriptor = open("bitmaps/font.tftf", O_RDONLY);
    fstat(file_descriptor, &statbuffer);
    font_buffer = (uint8_t*)malloc(sizeof(char) * statbuffer.st_size);
    read(file_descriptor, font_buffer, statbuffer.st_size);
    close(file_descriptor);
    font_header = (font_header_t*)font_buffer;
}

font_header_t* current_font_header()
{
    return font_header;
}
