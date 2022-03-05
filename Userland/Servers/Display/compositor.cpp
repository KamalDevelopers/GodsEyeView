#include "compositor.hpp"

Compositor::Compositor()
{
    display_framebuffer = request_framebuffer();
    final_layer = request_canvas(SCREEN_WIDTH, SCREEN_HEIGHT);
    root_layer = request_canvas(SCREEN_WIDTH, SCREEN_HEIGHT);
    mouse_layer = request_canvas(MOUSE_WIDTH, MOUSE_HEIGHT);
    mouse_ghost_layer = request_canvas(MOUSE_WIDTH, MOUSE_HEIGHT);

    display_layer.width = SCREEN_WIDTH;
    display_layer.height = SCREEN_HEIGHT;
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
    for (uint32_t i = 0; i < layer_index; i++) {
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
    int file_descriptor = open((char*)file_name, O_RDONLY);
    if (file_descriptor < 0)
        return 0;

    int size = read(file_descriptor, canvas->framebuffer, canvas->size * sizeof(int32_t));
    close(file_descriptor);
    return size;
}

void Compositor::load_background_bitmap(const char* file_name)
{
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
    if (x + mouse_layer->width > SCREEN_WIDTH)
        mouse_layer->x = SCREEN_WIDTH - mouse_layer->width;
    if (y + mouse_layer->height > SCREEN_HEIGHT)
        mouse_layer->y = SCREEN_HEIGHT - mouse_layer->height;

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
    layers[layer_index] = canvas;
    layer_index++;
}

int Compositor::remove_render_layer(canvas_t* canvas)
{
    int index = -1;
    for (uint32_t i = 0; i < layer_index; i++)
        if (canvas == layers[i])
            index = i;

    if (index == -1)
        return -1;

    delete_element(index, layer_index, layers);
    layer_index--;
    return 0;
}
