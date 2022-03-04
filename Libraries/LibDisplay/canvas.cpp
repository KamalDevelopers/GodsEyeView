#include "canvas.hpp"

canvas_t* request_canvas(uint32_t width, uint32_t height)
{
    canvas_t* canvas = (canvas_t*)malloc(sizeof(canvas_t));
    canvas->size = height * width;
    canvas->framebuffer = (uint32_t*)malloc(canvas->size * sizeof(int32_t));
    canvas->x = 0;
    canvas->y = 0;
    canvas->width = width;
    canvas->height = height;
    return canvas;
}

int request_canvas_destroy(canvas_t* canvas)
{
    if ((canvas->framebuffer == 0) || (canvas == 0))
        return -1;
    free(canvas->framebuffer);
    free(canvas);
    return 0;
}

int request_canvas_resize(canvas_t* canvas, uint32_t width, uint32_t height)
{
    uint32_t previous_size = canvas->size;
    canvas->size = height * width;
    canvas->width = width;
    canvas->height = height;

    if (canvas->size <= previous_size)
        return 0;

    if (canvas->framebuffer != 0)
        free(canvas->framebuffer);
    canvas->framebuffer = (uint32_t*)malloc(canvas->size * sizeof(int32_t));
    return 0;
}

uint32_t request_framebuffer()
{
    int fd = open("/dev/display");
    uint32_t buffer[1];
    if (read(fd, buffer, sizeof(uint32_t)))
        return buffer[0];
    return 0;
}

void canvas_copy_alpha(uint32_t* destination, uint32_t* source, int size)
{
    for (uint32_t i = 0; i < size; i++) {
        if (source[i] >= 0xFF000000)
            continue;
        if (destination[i] >= 0xFF000000)
            continue;
        destination[i] = source[i];
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
