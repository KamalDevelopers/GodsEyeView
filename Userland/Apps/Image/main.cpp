#include <LibC/path.h>
#include <LibC/poll.h>
#include <LibC/stdlib.h>
#include <LibDisplay/connection.hpp>
#include <LibDisplay/events.hpp>
#include <LibImage/png.h>
#include <LibImage/svg.h>

int init(canvas_t& window_canvas)
{
    int window_events_file = request_display_window(window_canvas, 500, 500, 0x000000);
    canvas_set(window_canvas.framebuffer, 0x000000, window_canvas.size);
    request_update_window();
    return window_events_file;
}

void uninit()
{
    request_destroy_window();
}

int read_bitmap(const char* file_name, canvas_t* canvas)
{
    int file_descriptor = open(file_name, O_RDONLY);
    if (file_descriptor < 0)
        return 0;

    int size = read(file_descriptor, canvas->framebuffer, canvas->size * sizeof(int32_t));
    close(file_descriptor);
    return size;
}

void draw(canvas_t* window_canvas, canvas_t* image_canvas)
{
    /* TODO: scaling */
    if (window_canvas->height < image_canvas->height || window_canvas->width < image_canvas->width)
        return;

    image_canvas->y = window_canvas->height / 2 - image_canvas->height / 2;
    image_canvas->x = window_canvas->width / 2 - image_canvas->width / 2;
    canvas_set(window_canvas->framebuffer, 0x000000, window_canvas->size);
    canvas_blit(window_canvas, image_canvas);
}

bool resize_window(display_event_t* display_event, canvas_t& window_canvas, canvas_t& image_canvas)
{
    window_canvas = display_event->canvas;
    draw(&window_canvas, &image_canvas);
    request_update_window();
    return false;
}

bool receive_keyboard_event(display_event_t* display_event)
{
    keyboard_event_t keyboard_event;
    memcpy(&keyboard_event, &display_event->keyboard, sizeof(keyboard_event_t));
    if (keyboard_event.key == 27)
        return true;
    return false;
}

bool receive_events(canvas_t& window_canvas, canvas_t& image_canvas)
{
    display_event_t display_event;
    if (receive_window_event(&display_event)) {
        if (display_event.type == DISPLAY_EVENT_KEYBOARD)
            return receive_keyboard_event(&display_event);
        if (display_event.type == DISPLAY_EVENT_RESIZE)
            return resize_window(&display_event, window_canvas, image_canvas);
    }
    return false;
}

int main(int argc, char** argv)
{
    if (argc < 1) {
        printf("Usage: image <file> [width] [height]\n");
        return 0;
    }

    char file[100];
    bool is_supported = false;
    memset(file, 0, 100);
    strcpy(file, argv[0]);
    char* token = strtok(argv[0], ".");
    char* new_token = token;
    int last_token_size = 0;
    int file_format = 0;

    while (new_token != NULL) {
        token = new_token;
        new_token = strtok(NULL, ".");
    }

    if (strncmp(token, "svg", 3) == 0) {
        file_format = 1;
        is_supported = true;
    }
    if (strncmp(token, "raw", 3) == 0) {
        is_supported = true;
        file_format = 2;
        if (argc < 3) {
            printf("File format 'raw' requires specifying image width and height\n");
            return 0;
        }
    }
    if (strncmp(token, "png", 3) == 0) {
        file_format = 3;
        is_supported = true;
    }

    if (!is_supported) {
        printf("File '%s' format unsupported\n", file);
        return 0;
    }

    canvas_t* image_canvas = 0;

    if (file_format == 3) {
        static png_image_t png_image;
        if (decode_png(file, &png_image) < 0) {
            printf("File '%s' does not exist or unsupported\n", file);
            return 0;
        }

        image_canvas = request_canvas(png_image.width, png_image.height);
        memcpy32(image_canvas->framebuffer, png_image.buffer, png_image.width * png_image.height * 4);
        free_png(&png_image);
    }

    if (file_format == 2) {
        int width = atoi(argv[1]);
        int height = atoi(argv[2]);
        image_canvas = request_canvas(width, height);
        if (read_bitmap(file, image_canvas) == 0) {
            printf("File '%s' does not exist\n", file);
            request_canvas_destroy(image_canvas);
            return 0;
        }
    }

    if (file_format == 1) {
        static svg_image_t svg_image;
        if (decode_svg(file, 0, &svg_image) < 0) {
            printf("File '%s' does not exist or unsupported\n", file);
            return 0;
        }

        image_canvas = request_canvas(svg_image.width, svg_image.height);
        memcpy32(image_canvas->framebuffer, svg_image.buffer, svg_image.width * svg_image.height * 4);
        free_svg(&svg_image);
    }

    canvas_t window_canvas;
    int window_events_file = init(window_canvas);
    struct pollfd polls[2];
    polls[0].events = POLLIN;
    polls[0].fd = 1;
    polls[1].events = POLLIN;
    polls[1].fd = window_events_file;
    draw(&window_canvas, image_canvas);
    request_update_window();

    while (1) {
        poll(polls, 2, 0);
        if (receive_events(window_canvas, *image_canvas))
            break;
    }

    request_canvas_destroy(image_canvas);
    uninit();
    return 0;
}
