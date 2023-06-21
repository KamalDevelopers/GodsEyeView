#include "canvas.hpp"
#include "gaussian.hpp"

#define GET_ALPHA(color) ((color >> 24) & 0x000000FF)
#define GET_RED(color) ((color >> 16) & 0x000000FF)
#define GET_GREEN(color) ((color >> 8) & 0x000000FF)
#define GET_BLUE(color) ((color >> 0) & 0X000000FF)

canvas_t* request_canvas(uint32_t width, uint32_t height)
{
    canvas_t* canvas = (canvas_t*)malloc(sizeof(canvas_t));
    canvas->size = height * width;
    canvas->framebuffer = (uint32_t*)malloc(canvas->size * sizeof(int32_t));
    canvas->x = 0;
    canvas->y = 0;
    canvas->width = width;
    canvas->height = height;
    canvas->alpha_lookup = 0;
    canvas->hidden = 0;
    return canvas;
}

int request_canvas_destroy(canvas_t* canvas)
{
    if ((canvas->framebuffer == 0) || (canvas == 0))
        return -1;

    if (canvas->alpha_lookup != 0)
        free(canvas->alpha_lookup);

    free(canvas->framebuffer);
    free(canvas);
    return 0;
}

int request_canvas_resize(canvas_t* canvas, uint32_t width, uint32_t height)
{
    uint32_t* fb = canvas->framebuffer;
    uint32_t previous_size = canvas->size;
    canvas->size = height * width;
    canvas->width = width;
    canvas->height = height;

    if (canvas->size <= previous_size)
        return 0;

    canvas->framebuffer = (uint32_t*)malloc(canvas->size * sizeof(int32_t));
    canvas_copy(canvas->framebuffer, fb, previous_size);
    canvas_set(canvas->framebuffer + previous_size, 0, canvas->size - previous_size);

    if (fb != 0)
        free(fb);
    return 0;
}

int request_framebuffer(uint32_t* framebuffer, uint32_t* width, uint32_t* height)
{
    int fd = open("/dev/display", O_RDONLY);
    uint32_t buffer[3];
    if (read(fd, buffer, sizeof(uint32_t))) {
        *framebuffer = buffer[0];
        *width = buffer[1];
        *height = buffer[2];
        return 0;
    }
    return 1;
}

void canvas_blur(uint32_t* destination, int size, int width, int height, uint32_t blur_level)
{
    int pixels = 0;

    for (uint32_t i = 1; i < size - 1; i++) {
        uint32_t nred = gaussian_blur_pixel(destination, i, width, 16, blur_level);
        uint32_t ngreen = gaussian_blur_pixel(destination, i, width, 8, blur_level);
        uint32_t nblue = gaussian_blur_pixel(destination, i, width, 0, blur_level);
        destination[i] = (0 << 24) | (nred << 16) | (ngreen << 8) | (nblue << 0);

        pixels++;
        if (pixels >= size)
            return;
    }
}

void canvas_copy_alpha(uint32_t* destination, uint32_t* source, int size, alpha_lookup_t* lookup)
{
    for (uint32_t i = 0; i < size; i++) {
        uint16_t alpha = GET_ALPHA(source[i]);

        if (alpha == 0) {
            destination[i] = source[i];
            continue;
        }

        if (alpha == 255)
            continue;

        destination[i] = pixel_alpha_blend(source[i], destination[i], 255 - alpha, lookup);
    }
}

void canvas_copy(uint32_t* destination, uint32_t* source, int size)
{
    memcpy32(destination, source, size * sizeof(int32_t));
}

void canvas_set(uint32_t* destination, uint32_t rgb, int size)
{
    for (uint32_t i = 0; i < size; i++)
        destination[i] = rgb;
}

void canvas_copy(canvas_t* destination, canvas_t* source)
{
    uint32_t source_offset = destination->y * source->width + destination->x;
    uint32_t destination_offset = 0;

    for (uint32_t y = 0; y < destination->height; y++) {
        canvas_copy(destination->framebuffer + destination_offset, source->framebuffer + source_offset, destination->width);
        source_offset += source->width;
        destination_offset += destination->width;
    }
}

void canvas_blit(canvas_t* destination, canvas_t* source)
{
    int destination_pitch = destination->width * sizeof(int32_t);
    int source_pitch = source->width * sizeof(int32_t);
    uint32_t destination_address = source->y * destination_pitch + source->x * sizeof(int32_t) + (uint32_t)destination->framebuffer;
    uint32_t source_address = (uint32_t)(source->framebuffer);

    for (uint32_t y = 0; y < source->height; y++) {
        memcpy32((uint32_t*)destination_address, (uint32_t*)source_address, source_pitch);
        source_address += source_pitch;
        destination_address += destination_pitch;
    }
}

void canvas_create_alpha(canvas_t* canvas, uint32_t color)
{
    if (canvas->alpha_lookup)
        free(canvas->alpha_lookup);

    canvas->alpha_lookup = (alpha_lookup_t*)malloc(sizeof(alpha_lookup_t));
    uint32_t alpha = GET_ALPHA(color);
    for (uint16_t y = 0; y < 256; y++) {
        for (uint16_t x = 0; x < 256; x++) {
            canvas->alpha_lookup->table[y][x] = y + (alpha * 1.0 / 255) * x;
        }
    }
}

uint32_t pixel_alpha_blend(uint32_t fg, uint32_t bg, uint32_t alpha1, alpha_lookup_t* lookup)
{
    uint32_t red1 = GET_RED(fg);
    uint32_t green1 = GET_GREEN(fg);
    uint32_t blue1 = GET_BLUE(fg);

    uint32_t alpha2 = 255 - GET_ALPHA(bg);
    uint32_t red2 = GET_RED(bg);
    uint32_t green2 = GET_GREEN(bg);
    uint32_t blue2 = GET_BLUE(bg);

    if (lookup) {
        return (lookup->table[red1][red2] << 16) | (lookup->table[green1][green2] << 8) | lookup->table[blue1][blue2];
    }

    uint32_t r = (uint32_t)((alpha1 * 1.0 / 255) * red1);
    uint32_t g = (uint32_t)((alpha1 * 1.0 / 255) * green1);
    uint32_t b = (uint32_t)((alpha1 * 1.0 / 255) * blue1);

    r = r + (((255 - alpha1) * 1.0 / 255) * (alpha2 * 1.0 / 255)) * red2;
    g = g + (((255 - alpha1) * 1.0 / 255) * (alpha2 * 1.0 / 255)) * green2;
    b = b + (((255 - alpha1) * 1.0 / 255) * (alpha2 * 1.0 / 255)) * blue2;

    uint32_t new_alpha = (uint32_t)(alpha1 + ((255 - alpha1) * 1.0 / 255) * alpha2);
    uint32_t color1_over_color2 = ((255 - new_alpha) << 24) | (r << 16) | (g << 8) | (b << 0);
    return color1_over_color2;
}
