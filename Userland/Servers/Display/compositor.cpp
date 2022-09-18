#include "compositor.hpp"
#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/string.h>
#include <LibC/unistd.h>

Compositor::Compositor()
{
    request_framebuffer(&display_framebuffer, &display_width, &display_height);
    final_layer = request_canvas(display_width, display_height);
    root_layer = request_canvas(display_width, display_height);
    mouse_layer = request_canvas(MOUSE_WIDTH, MOUSE_HEIGHT);
    mouse_ghost_layer = request_canvas(MOUSE_WIDTH, MOUSE_HEIGHT);
    canvas_set(root_layer->framebuffer, 0, root_layer->size);

    display_layer.width = display_width;
    display_layer.height = display_height;
    display_layer.x = 0;
    display_layer.y = 0;
    display_layer.framebuffer = (uint32_t*)display_framebuffer;
}

Compositor::~Compositor()
{
    request_canvas_destroy(root_layer);
    request_canvas_destroy(mouse_layer);
    request_canvas_destroy(final_layer);
    request_canvas_destroy(mouse_ghost_layer);
    free(mouse_bitmap);
}

void Compositor::render_canvas(canvas_t* canvas)
{
    uint32_t final_layer_address = (uint32_t)(final_layer->framebuffer) + canvas->x * sizeof(int32_t);
    final_layer_address += canvas->y * (final_layer->width * sizeof(int32_t));
    uint32_t canvas_address = (uint32_t)(canvas->framebuffer);

    for (uint32_t y = 0; y < canvas->height; y++) {
        if (canvas->alpha_lookup)
            canvas_copy_alpha((uint32_t*)final_layer_address, (uint32_t*)canvas_address, canvas->width, canvas->alpha_lookup);
        else
            canvas_copy((uint32_t*)final_layer_address, (uint32_t*)canvas_address, canvas->width);

        canvas_address += canvas->width * sizeof(int32_t);
        final_layer_address += final_layer->width * sizeof(int32_t);
    }
}

void Compositor::render_borders(canvas_t* canvas)
{
    uint32_t border_color = canvas->border_decoration;
    if (!border_color)
        return;

    canvas_set(canvas->framebuffer, border_color, canvas->width);
    canvas_set(canvas->framebuffer + canvas->size - canvas->width, border_color, canvas->width);
    for (uint32_t i = 0; i < canvas->size; i += canvas->width) {
        canvas->framebuffer[i] = border_color;
        canvas->framebuffer[i + canvas->width - 1] = border_color;
    }
}

void Compositor::render_stack()
{
    if (!needs_update)
        return;

    /* TODO: Make this faster by reducing draw areas! */
    render_canvas(root_layer);
    for (uint32_t i = 0; i < layers.size(); i++) {
        render_borders(layers[i]);
        render_canvas(layers[i]);
    }

    update_mouse_position(mouse_layer->x, mouse_layer->y, true);
    canvas_copy((uint32_t*)display_framebuffer, final_layer->framebuffer, final_layer->size);
    canvas_blit(final_layer, mouse_ghost_layer);
    needs_update = false;
}

int Compositor::read_bitmap(const char* file_name, canvas_t* canvas)
{
    int file_descriptor = open(file_name, O_RDONLY);
    if (file_descriptor < 0)
        return 0;

    int size = read(file_descriptor, canvas->framebuffer, canvas->size * sizeof(int32_t));
    close(file_descriptor);
    return size;
}

int Compositor::read_compressed_bitmap(const char* file_name, canvas_t* canvas)
{
    int file_descriptor = open(file_name, O_RDONLY);
    if (file_descriptor < 0)
        return 0;

    struct stat statbuffer;
    fstat(file_descriptor, &statbuffer);
    uint32_t* buffer = (uint32_t*)malloc(statbuffer.st_size);
    read(file_descriptor, buffer, statbuffer.st_size);
    close(file_descriptor);

    size_t pixel_count = statbuffer.st_size / sizeof(uint32_t);
    uint32_t* frame = canvas->framebuffer;
    uint32_t last_pixel = 0;

    for (uint32_t i = 0; i < pixel_count; i++) {
        uint32_t pixel = buffer[i];
        uint32_t repeat = (pixel >> 24) & 0x000000FF;
        for (uint32_t r = 0; r < repeat; r++) {
            *frame = last_pixel;
            frame++;
        }

        *frame = pixel;
        frame++;
        last_pixel = pixel;
    }

    free(buffer);
    return statbuffer.st_size;
}

void Compositor::load_background_bitmap(const char* file_name, bool is_compressed)
{
    if (is_compressed) {
        printf("%d\n", read_compressed_bitmap(file_name, root_layer));
        return;
    }

    read_bitmap(file_name, root_layer);
}

void Compositor::load_mouse_bitmap(const char* file_name)
{
    read_bitmap(file_name, mouse_layer);
    mouse_bitmap = (uint32_t*)malloc(mouse_layer->size * sizeof(int32_t));
    canvas_copy(mouse_bitmap, mouse_layer->framebuffer, mouse_layer->size);
}

void Compositor::update_mouse_position(uint32_t x, uint32_t y, bool is_updating_stack)
{
    if ((x == mouse_layer->x) && (y == mouse_layer->y) && !is_updating_stack)
        return;

    mouse_layer->x = x;
    mouse_layer->y = y;
    if (x + mouse_layer->width > display_width)
        mouse_layer->x = display_width - mouse_layer->width;
    if (y + mouse_layer->height > display_height)
        mouse_layer->y = display_height - mouse_layer->height;

    canvas_copy(mouse_layer, final_layer);
    mouse_ghost_layer->x = mouse_layer->x;
    mouse_ghost_layer->y = mouse_layer->y;
    canvas_copy(mouse_ghost_layer, final_layer);
    canvas_copy_alpha(mouse_layer->framebuffer, mouse_bitmap, mouse_layer->size);
    canvas_blit(final_layer, mouse_layer);

    if (!is_updating_stack) {
        canvas_copy((uint32_t*)display_framebuffer, final_layer->framebuffer, final_layer->size);
        canvas_blit(final_layer, mouse_ghost_layer);
    }
}

void Compositor::add_render_layer(canvas_t* canvas)
{
    layers.append(canvas);
}

int Compositor::remove_render_layer(canvas_t* canvas)
{
    int index = -1;
    for (uint32_t i = 0; i < layers.size(); i++)
        if (canvas == layers[i])
            index = i;

    if (index == -1)
        return -1;

    layers.remove_at(index);
    return 0;
}
