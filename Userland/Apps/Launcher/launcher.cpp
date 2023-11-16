#include "launcher.hpp"

Launcher::Launcher()
{
    uint32_t fb = 0;
    /* Get the screen size */
    request_framebuffer(&fb, &width, &height);
    height = 17;

    has_spawned_children = false;
    uint8_t flags = 0 | DISPLAY_FLAG_DISOWNED;
    window_events_file = request_display_window(window_canvas, width, height, bg, flags);
    canvas_set(window_canvas.framebuffer, bg, window_canvas.size);
    default_font = font_load("bitmaps/font.tftf");

    request_update_window();
}

Launcher::~Launcher()
{
    request_destroy_window();
    font_unload(default_font);
}

void Launcher::resize_window(display_event_t* display_event)
{
    window_canvas = display_event->canvas;
    /* request_update_window(); */
}

void Launcher::receive_keyboard_event(display_event_t* display_event)
{
    keyboard_event_t keyboard_event;
    memcpy(&keyboard_event, &display_event->keyboard, sizeof(keyboard_event_t));
}

void Launcher::receive_events()
{
    display_event_t display_event;
    if (receive_window_event(&display_event)) {
        if (display_event.type == DISPLAY_EVENT_KEYBOARD)
            return receive_keyboard_event(&display_event);
        if (display_event.type == DISPLAY_EVENT_RESIZE)
            return resize_window(&display_event);
    }
}

uint32_t Launcher::display_string(const char* text, int pos_x, int pos_y)
{
    size_t size = strlen(text);
    for (uint32_t i = 0; i < size; i++)
        pos_x = font_display_character_with_bg(default_font, &window_canvas, text[i], pos_x, pos_y, fg, bg, false);
    return pos_x;
}

static const char months[12][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void Launcher::display_time()
{
    uint32_t timestamp = time();
    uint32_t tcopy = timestamp;
    unsigned sec = tcopy % 60;
    tcopy /= 60;
    unsigned min = tcopy % 60;
    tcopy /= 60;
    unsigned hour = tcopy % 24;

    int z = timestamp / 86400 + 719468;
    int era = (z >= 0 ? z : z - 146096) / 146097;
    unsigned doe = static_cast<unsigned>(z - era * 146097);
    unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int y = static_cast<int>(yoe) + era * 400;
    unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    unsigned mp = (5 * doy + 2) / 153;
    unsigned d = doy - (153 * mp + 2) / 5 + 1;
    unsigned m = mp + (mp < 10 ? 3 : -9);
    y += (m <= 2);

    char minutes[10];
    memset(&minutes, 0, 10);
    itoa(min, minutes);
    char hours[10];
    memset(&hours, 0, 10);
    itoa(hour, hours);
    char day[10];
    memset(&day, 0, 10);
    itoa(y, day);

    int pos_x = width - 150;
    int pos_y = 6;
    pos_x = display_string(" ", pos_x, pos_y);
    pos_x = display_string(months[m - 1], pos_x, pos_y);
    pos_x = display_string(" ", pos_x, pos_y);
    pos_x = display_string(day, pos_x, pos_y);
    pos_x = display_string(" ", pos_x, pos_y);
    if (hour < 10)
        pos_x = display_string("0", pos_x, pos_y);
    pos_x = display_string(hours, pos_x, pos_y);
    pos_x = display_string(":", pos_x, pos_y);
    if (min < 10)
        pos_x = display_string("0", pos_x, pos_y);
    pos_x = display_string(minutes, pos_x, pos_y);
}

void Launcher::display_cpu_usage()
{
    struct osinfo info;
    sys_osinfo(&info);
    double cpu_ticks = info.cpu_usage_ticks;
    double cpu_max_ticks = 10000;
    double usage = cpu_ticks / cpu_max_ticks * 100;
    uint32_t number = ((uint32_t)usage > 100) ? 100 : (uint32_t)usage;

    char usage_string[3];
    memset(&usage_string, 0, 10);
    itoa(number, usage_string);

    int pos_x = 18;
    int pos_y = 6;

    pos_x = display_string("cpu:", pos_x, pos_y);

    if (number < 100) {
        display_string(" ", pos_x, pos_y - 5);
        pos_x = display_string(" ", pos_x, pos_y);
    }
    if (number < 10) {
        display_string(" ", pos_x, pos_y - 5);
        pos_x = display_string(" ", pos_x, pos_y);
    }
    pos_x = display_string(usage_string, pos_x, pos_y);
    pos_x = display_string("%", pos_x, pos_y);
}

void Launcher::spawns()
{
    spawn("bin/terminal", 0, 0);
    has_spawned_children = true;
}

void Launcher::run()
{
    is_running = true;
    struct pollfd polls[1];
    polls[0].events = POLLIN;
    polls[0].fd = window_events_file;

    while (1) {
        canvas_set(window_canvas.framebuffer, bg, window_canvas.size);
        display_time();
        display_cpu_usage();
        request_update_window();

        if (!has_spawned_children)
            spawns();

        poll(polls, 1, 2700);
        receive_events();
        request_update_window();

        if (!is_running)
            return;
    }
}
