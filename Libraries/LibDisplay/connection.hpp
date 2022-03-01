#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <LibC/stdio.hpp>
#include <LibC/string.hpp>
#include <LibC/types.hpp>
#include <LibC/unistd.hpp>
#include <LibDisplay/canvas.hpp>
#include <LibDisplay/events.hpp>

#define DISPLAY_CREATE 1
#define DISPLAY_UPDATE 2
#define DISPLAY_DESTROY 3

#define DISPLAY_EVENT_RESPONSE 1
#define DISPLAY_EVENT_RESIZE 2
#define DISPLAY_EVENT_MOUSE 3
#define DISPLAY_EVENT_KEYBOARD 4

typedef struct display_request {
    int pid = -1;
    int type = 0;
    uint32_t width = 0;
    uint32_t height = 0;
} display_request_t;

typedef struct display_event {
    int type = 0;
    mouse_event_t mouse;
    keyboard_event_t keyboard;
    canvas_t canvas;
} display_event_t;

int request_display_window(canvas_t& canvas, uint32_t width, uint32_t height);
void request_update_window();
void request_destroy_window();
bool receive_event(display_event_t* event);

#endif
