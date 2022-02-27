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
    /* FIXME: Resize framebuffer */
    /*
     * if (canvas->framebuffer != 0)
     *     free(canvas->framebuffer);
     * canvas->framebuffer = (uint32_t*)malloc(canvas->size * sizeof(int32_t));
     */

    canvas->size = height * width;
    canvas->width = width;
    canvas->height = height;
    return 0;
}

int request_canvas_update(canvas_t* canvas)
{
    int status;
    asm volatile("int $0x80"
                 : "=a"(status)
                 : "a"(404), "b"(canvas), "c"(1));
    return status;
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

void canvas_copy(canvas_t* child, canvas_t* parent)
{
    uint32_t parent_offset = child->y * parent->width + child->x;
    uint32_t child_offset = 0;

    for (uint32_t y = 0; y < child->height; y++) {
        canvas_copy(child->framebuffer + child_offset, parent->framebuffer + parent_offset, child->width);
        parent_offset += parent->width;
        child_offset += child->width;
    }
}
