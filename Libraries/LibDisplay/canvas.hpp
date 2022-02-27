#ifndef CANVAS_HPP
#define CANVAS_HPP

#include <LibC/stdlib.hpp>

typedef struct canvas {
    uint32_t* framebuffer = 0;
    uint32_t size = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t x = 0;
    uint32_t y = 0;
} canvas_t;

canvas_t* request_canvas(uint32_t width, uint32_t height);
int request_canvas_resize(canvas_t* canvas, uint32_t width, uint32_t height);
int request_canvas_update(canvas_t* canvas);
int request_canvas_destroy(canvas_t* canvas);
void canvas_copy_alpha(uint32_t* destination, uint32_t* source, int size);
void canvas_copy(uint32_t* destination, uint32_t* source, int size);
void canvas_copy(canvas_t* child, canvas_t* parent);
void canvas_set(uint32_t* destination, uint32_t rgb, int size);

#endif
