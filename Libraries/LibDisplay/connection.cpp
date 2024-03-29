#include "connection.hpp"
#include <LibC/exit.h>

static int events_file = -1;
static int display_file = -1;

static void display_exit()
{
    request_destroy_window();
}

int request_display_window(canvas_t& canvas, uint32_t width, uint32_t height, uint32_t bg, uint8_t flags)
{
    display_request_t request;
    request.pid = getpid();
    request.type = DISPLAY_CREATE;
    request.width = width;
    request.height = height;
    request.flags = flags;
    request.bg = bg;

    display_file = -1;
    while (display_file < 0) {
        display_file = open("/pipe/display", O_RDWR | O_APPEND);
        usleep(5);
    }

    write(display_file, &request, sizeof(display_request_t));

    display_event_t event;
    char events_file_name[50];
    char pid[10];
    memset(pid, 0, sizeof(pid));
    memset(events_file_name, 0, sizeof(events_file_name));
    itoa(request.pid, pid);
    strcat(events_file_name, "/pipe/display-events-");
    strcat(events_file_name, pid);

    events_file = -1;
    while (events_file < 0) {
        events_file = open(events_file_name, O_RDWR);
        usleep(5);
    }

    int read_size = 0;
    while (read_size < 1) {
        read_size = read(events_file, (void*)&event, sizeof(display_event_t));
        if (event.type == DISPLAY_EVENT_RESPONSE) {
            canvas = event.canvas;
            atexit(&display_exit);
            return events_file;
        }
        usleep(5);
    }

    return -1;
}

void request_update_window()
{
    if (display_file == -1)
        return;

    display_request_t request;
    request.pid = getpid();
    request.type = DISPLAY_UPDATE;
    write(display_file, &request, sizeof(display_request_t));
}

void request_destroy_window()
{
    if (display_file == -1)
        return;

    display_request_t request;
    request.pid = getpid();
    request.type = DISPLAY_DESTROY;
    write(display_file, &request, sizeof(display_request_t));
}

bool receive_window_event(display_event_t* event)
{
    if (read(events_file, event, sizeof(display_event_t)))
        return true;
    return false;
}
