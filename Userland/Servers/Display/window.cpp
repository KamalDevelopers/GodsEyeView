#include "window.hpp"
#include "compositor.hpp"

Window::Window(int pid)
{
    associated_pid = pid;
}

Window::~Window()
{
    request_canvas_destroy(canvas);
    close(process_send_event_file);
}

void Window::set_position(uint32_t x, uint32_t y)
{
    canvas->x = x;
    canvas->y = y;
}

void Window::resize(uint32_t width, uint32_t height)
{
    if (canvas == 0) {
        canvas = request_canvas(width, height);
        canvas_set(canvas->framebuffer, 0, canvas->size);
    }

    if ((width == canvas->width) && (height == canvas->height))
        return;

    request_canvas_resize(canvas, width, height);
    resize_event(canvas);
}

void Window::disown()
{
    controlled = false;
}

void Window::create_process_connection()
{
    if (associated_pid == -1)
        return;

    char events_file_name[50];
    char pid[10];

    memset(events_file_name, 0, sizeof(events_file_name));
    memset(pid, 0, sizeof(pid));
    itoa(associated_pid, pid);
    strcat(events_file_name, (char*)"/pipe/display-events-");
    strcat(events_file_name, pid);
    process_send_event_file = mkfifo(events_file_name, O_RDWR | O_APPEND);

    display_event_t event;
    event.type = DISPLAY_EVENT_RESPONSE;
    memcpy(&event.canvas, canvas, sizeof(canvas_t));
    write(process_send_event_file, &event, sizeof(display_event_t));
}

bool Window::is_point_in_window(uint32_t x, uint32_t y)
{
    if ((x < canvas->x) || (y < canvas->y))
        return false;
    if ((x > canvas->x + canvas->width) || (y > canvas->y + canvas->height))
        return false;
    return true;
}

void Window::resize_event(canvas_t* canvas)
{
    display_event_t event;
    event.type = DISPLAY_EVENT_RESIZE;
    memcpy(&event.canvas, canvas, sizeof(canvas_t));
    write(process_send_event_file, &event, sizeof(display_event_t));
}

void Window::mouse_event(mouse_event_t* event)
{
    if (!is_point_in_window(event->x, event->y))
        return;

    display_event_t send_event;
    send_event.type = DISPLAY_EVENT_MOUSE;
    memcpy(&send_event.mouse, event, sizeof(mouse_event_t));
    write(process_send_event_file, &send_event, sizeof(display_event_t));
}

void Window::keyboard_event(keyboard_event_t* event)
{
    display_event_t send_event;
    send_event.type = DISPLAY_EVENT_KEYBOARD;
    memcpy(&send_event.keyboard, event, sizeof(keyboard_event_t));
    write(process_send_event_file, &send_event, sizeof(display_event_t));
}
