#include "compositor.hpp"
#include <LibC/math.h>
#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/string.h>
#include <LibC/unistd.h>
#include <LibImage/png.h>
#include <LibImage/svg.h>

Compositor::Compositor()
{
    request_framebuffer(&display_framebuffer, &display_width, &display_height);
    final_layer = request_canvas(display_width, display_height);
    blured_final_layer = request_canvas(display_width, display_height);
    root_layer = request_canvas(display_width, display_height);
    mouse_layer = request_canvas(MOUSE_WIDTH, MOUSE_HEIGHT);
    mouse_ghost_layer = request_canvas(MOUSE_WIDTH, MOUSE_HEIGHT);
    canvas_set(root_layer->framebuffer, 0, root_layer->size);

    display_layer.width = display_width;
    display_layer.height = display_height;
    display_layer.x = 0;
    display_layer.y = 0;
    display_layer.framebuffer = (uint32_t*)display_framebuffer;
    has_blured_final_layer = 0;
}

Compositor::~Compositor()
{
    request_canvas_destroy(root_layer);
    request_canvas_destroy(mouse_layer);
    request_canvas_destroy(final_layer);
    request_canvas_destroy(blured_final_layer);
    request_canvas_destroy(mouse_ghost_layer);
    free(mouse_bitmap);
}

void Compositor::render_canvas(canvas_t* canvas)
{
    uint32_t offset = canvas->x * sizeof(int32_t);
    offset += canvas->y * (final_layer->width * sizeof(int32_t));
    uint32_t canvas_address = (uint32_t)(canvas->framebuffer);

    for (uint32_t y = 0; y < canvas->height; y++) {
        if (canvas->alpha_lookup) {
            canvas_copy((uint32_t*)(offset + (uint32_t)final_layer->framebuffer), (uint32_t*)(offset + (uint32_t)blured_final_layer->framebuffer), canvas->width);
            canvas_copy_alpha((uint32_t*)(offset + (uint32_t)final_layer->framebuffer), (uint32_t*)canvas_address, canvas->width, canvas->alpha_lookup);
        } else {
            canvas_copy((uint32_t*)(offset + (uint32_t)final_layer->framebuffer), (uint32_t*)canvas_address, canvas->width);
        }

        canvas_address += canvas->width * sizeof(int32_t);
        offset += final_layer->width * sizeof(int32_t);
    }
}

void Compositor::create_blur_layer()
{
    canvas_copy(blured_final_layer, root_layer);
    uint32_t address = 0;
    uint32_t max = final_layer->height - 65;
    uint32_t offset = final_layer->width * sizeof(int32_t);
    uint32_t start = offset * 15 + 40;

    address = (uint32_t)(blured_final_layer->framebuffer);
    for (uint32_t y = 30; y < max; y++) {
        canvas_blur_box((uint32_t*)address + start, final_layer->width - 80, final_layer->width, final_layer->height, 4);
        address += offset;
    }

    address = (uint32_t)(blured_final_layer->framebuffer);
    for (uint32_t y = 30; y < max; y++) {
        canvas_blur_gaussian((uint32_t*)address + start, final_layer->width - 80, final_layer->width, final_layer->height, 4096 * 2);
        address += offset;
    }
    has_blured_final_layer = 1;
}

void Compositor::render_rounded_borders(canvas_t* canvas)
{
    int radius = 20;
    uint32_t border_color = canvas->border_decoration;
    int pow_radius = radius * radius;
    if ((!border_color) || (canvas->height <= radius * 2) || (!canvas->alpha_lookup))
        return;

    for (uint32_t i = 1; i < radius; i++) {
        uint32_t offset = sqrt(pow_radius - (pow(radius - i, 2)));
        uint32_t iw = canvas->width * i;
        uint32_t hw = canvas->width * canvas->height;
        uint32_t fill = 0xFFFFFFFF;

        canvas_set(canvas->framebuffer + iw, fill, radius - offset); // top left
        canvas->framebuffer[iw + radius - offset - 1] = border_color;
        canvas_set(canvas->framebuffer + (hw - iw), fill, radius - offset); // top right
        canvas->framebuffer[(hw - iw) + radius - offset - 1] = border_color;
        canvas_set(canvas->framebuffer + iw + canvas->width - radius + offset, fill, radius - offset); // bottom right
        canvas->framebuffer[iw + canvas->width - radius + offset] = border_color;
        canvas_set(canvas->framebuffer + (hw - iw) + canvas->width - radius + offset, fill, radius - offset); // bottom left
        canvas->framebuffer[(hw - iw) + canvas->width - radius + offset] = border_color;

        if (i < (ceil(radius / 2) + 5)) {
            canvas->framebuffer[iw + radius - offset - 2] = border_color;
            canvas->framebuffer[(hw - iw) + radius - offset - 2] = border_color;
            canvas->framebuffer[iw + canvas->width - radius + offset + 1] = border_color;
            canvas->framebuffer[(hw - iw) + canvas->width - radius + offset + 1] = border_color;
        }
        if (i < 4) {
            canvas->framebuffer[iw + radius - offset - 3] = border_color;
            canvas->framebuffer[(hw - iw) + radius - offset - 3] = border_color;
            canvas->framebuffer[iw + canvas->width - radius + offset + 2] = border_color;
            canvas->framebuffer[(hw - iw) + canvas->width - radius + offset + 2] = border_color;
        }
        if (i == 1) {
            canvas_set(canvas->framebuffer, fill, radius - offset);
            canvas_set(canvas->framebuffer + canvas->width - radius + offset + 1, fill, radius - offset - 1);
        }
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

bool Compositor::is_layer_visible(canvas_t* canvas)
{
    if (canvas->hidden)
        return 0;
    if (canvas->x + canvas->width > root_layer->width)
        return 0;
    if (canvas->y + canvas->height > root_layer->height)
        return 0;
    return 1;
}

void Compositor::render_single_layer(canvas_t* canvas)
{
    if (!is_layer_visible(canvas))
        return;

    uint32_t final_layer_address = (uint32_t)(final_layer->framebuffer) + canvas->x * sizeof(int32_t);
    final_layer_address += canvas->y * (final_layer->width * sizeof(int32_t));
    uint32_t canvas_address = (uint32_t)(root_layer->framebuffer) + canvas->x * sizeof(int32_t);
    canvas_address += canvas->y * (root_layer->width * sizeof(int32_t));

    for (uint32_t y = 0; y < canvas->height; y++) {
        canvas_copy((uint32_t*)final_layer_address, (uint32_t*)canvas_address, canvas->width);
        canvas_address += root_layer->width * sizeof(int32_t);
        final_layer_address += final_layer->width * sizeof(int32_t);
    }

    render_borders(canvas);
    render_rounded_borders(canvas);
    render_canvas(canvas);

    update_mouse_position(mouse_layer->x, mouse_layer->y, true);
    canvas_copy((uint32_t*)display_framebuffer, final_layer->framebuffer, final_layer->size);
    canvas_blit(final_layer, mouse_ghost_layer);
}

void Compositor::render_stack()
{
    if (!has_blured_final_layer)
        create_blur_layer();

    if (!needs_update && !update_canvas)
        return;

    if (!needs_update && update_canvas) {
        render_single_layer(update_canvas);
        update_canvas = 0;
        return;
    }

    /* TODO: Make this faster by reducing draw areas! */
    render_canvas(root_layer);
    for (uint32_t i = 0; i < layers.size(); i++) {
        if (!is_layer_visible(layers[i]))
            continue;
        render_borders(layers[i]);
        render_rounded_borders(layers[i]);
        render_canvas(layers[i]);
    }

    update_mouse_position(mouse_layer->x, mouse_layer->y, true);
    canvas_copy((uint32_t*)display_framebuffer, final_layer->framebuffer, final_layer->size);
    canvas_blit(final_layer, mouse_ghost_layer);
    needs_update = false;
}

void Compositor::require_update()
{
    needs_update = true;
}

void Compositor::require_update_next()
{
    next_needs_update = true;
}

void Compositor::require_update_canvas(canvas_t* canvas)
{
    if (next_needs_update) {
        needs_update = true;
        next_needs_update = false;
        return;
    }
    update_canvas = canvas;
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

int Compositor::read_svg(const char* file_name, canvas_t* canvas)
{
    has_blured_final_layer = false;
    static svg_image_t image;
    image.in_resize = (float)screen_width() / 1440.0f;
    if (decode_svg(file_name, 0, &image) < 0)
        return 0;
    int size = image.height * image.width * 4;
    if (size > canvas->size)
        size = canvas->size * 4;
    memcpy32(canvas->framebuffer, image.buffer, size);
    free_svg(&image);
    return size;
}

int Compositor::read_png(const char* file_name, canvas_t* canvas)
{
    has_blured_final_layer = false;
    static png_image_t image;
    if (decode_png(file_name, &image) < 0)
        return 0;
    int size = image.height * image.width * 4;
    if (size > canvas->size)
        size = canvas->size * 4;
    memcpy32(canvas->framebuffer, image.buffer, size);
    free_png(&image);
    return size;
}

int Compositor::load_background_png(const char* file_name)
{
    int ret = read_png(file_name, root_layer);
    has_blured_final_layer = false;
    return ret;
}

int Compositor::load_background_svg(const char* file_name)
{
    int ret = read_svg(file_name, root_layer);
    has_blured_final_layer = false;
    return ret;
}

int Compositor::load_background_bitmap(const char* file_name)
{
    int ret = read_bitmap(file_name, root_layer);
    has_blured_final_layer = false;
    return ret;
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
