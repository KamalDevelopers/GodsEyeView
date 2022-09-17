#include "font.hpp"

#define GET_ALPHA(color) ((color >> 24) & 0x000000FF)
#define GET_RED(color) ((color >> 16) & 0x000000FF)
#define GET_GREEN(color) ((color >> 8) & 0x000000FF)
#define GET_BLUE(color) ((color >> 0) & 0X000000FF)

static font_header_t* font_header = 0;
static uint8_t* font_buffer = 0;

static uint32_t pixel_alpha_blend(uint32_t fg, uint32_t bg, uint32_t alpha1)
{
    uint32_t red1 = GET_RED(fg);
    uint32_t green1 = GET_GREEN(fg);
    uint32_t blue1 = GET_BLUE(fg);

    uint32_t alpha2 = 255;
    uint32_t red2 = GET_RED(bg);
    uint32_t green2 = GET_GREEN(bg);
    uint32_t blue2 = GET_BLUE(bg);

    uint32_t r = (uint32_t)((alpha1 * 1.0 / 255) * red1);
    uint32_t g = (uint32_t)((alpha1 * 1.0 / 255) * green1);
    uint32_t b = (uint32_t)((alpha1 * 1.0 / 255) * blue1);

    r = r + (((255 - alpha1) * 1.0 / 255) * (alpha2 * 1.0 / 255)) * red2;
    g = g + (((255 - alpha1) * 1.0 / 255) * (alpha2 * 1.0 / 255)) * green2;
    b = b + (((255 - alpha1) * 1.0 / 255) * (alpha2 * 1.0 / 255)) * blue2;

    uint32_t new_alpha = (uint32_t)(alpha1 + ((255 - alpha1) * 1.0 / 255) * alpha2);
    uint32_t color1_over_color2 = (new_alpha << 24) | (r << 16) | (g << 8) | (b << 0);
    return color1_over_color2;
}

uint32_t font_display_character(canvas_t* canvas, char c, uint32_t pos_x,
    uint32_t pos_y, uint32_t color, uint32_t bg, bool use_bg)
{
    uint32_t offset = font_header->chars[c].ptr;
    pos_y += font_header->chars[c].y_offset;
    pos_x += font_header->chars[c].x_offset;
    uint32_t* ptr = canvas->framebuffer + pos_y * canvas->width + pos_x;
    uint8_t* font = font_buffer + offset;

    for (uint32_t y = 0; y < font_header->height; y++) {
        uint32_t* dptr = ptr;
        for (uint32_t x = 0; x < font_header->width; x++) {
            uint8_t alpha = *font;
            uint32_t orig = (use_bg) ? bg : *dptr;
            uint32_t pixel = pixel_alpha_blend(color, orig, alpha);
            *dptr = pixel;
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
