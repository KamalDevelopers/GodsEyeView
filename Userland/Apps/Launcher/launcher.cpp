#include "launcher.hpp"
#include "font.hpp"

uint8_t* load_font(char* name)
{
    struct stat statbuffer;
    int file_descriptor = open(name, O_RDONLY);
    fstat(file_descriptor, &statbuffer);
    uint8_t* font_buffer = (uint8_t*)malloc(sizeof(char) * statbuffer.st_size);

    read(file_descriptor, font_buffer, statbuffer.st_size);
    close(file_descriptor);
    return font_buffer;
}

void unload_font(uint8_t* font_buffer)
{
    if (font_buffer)
        free(font_buffer);
}

Launcher::Launcher()
{
    uint32_t fb = 0;
    /* Get the screen size */
    request_framebuffer(&fb, &width, &height);
    height = 17;

    uint8_t flags = 0 | DISPLAY_FLAG_DISOWNED;
    window_events_file = request_display_window(window_canvas, width, height, 0x080808, flags);
    canvas_set(window_canvas.framebuffer, 0x080808, window_canvas.size);
    font_buffer = load_font((char*)"bitmaps/ter-u12b.psfu");
    request_update_window();
}

Launcher::~Launcher()
{
    unload_font(font_buffer);
    request_destroy_window();
    kill(0, 2);
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
    for (uint32_t i = 0; i < strlen(text); i++) {
        display_character(&window_canvas, (psf_font_t*)font_buffer, text[i], pos_x, 2, 0x686868, 0x080808);
        pos_x += ((psf_font_t*)font_buffer)->width + 1;
    }
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
    itoa(min, minutes);
    char hours[10];
    itoa(hour, hours);
    char day[10];
    itoa(y, day);

    int pos_x = width - 122;
    pos_x = display_string("[", pos_x, 2);
    pos_x = display_string(months[m - 1], pos_x, 2);
    pos_x = display_string(" ", pos_x, 2);
    pos_x = display_string(day, pos_x, 2);
    pos_x = display_string(" ", pos_x, 2);
    if (hour < 10)
        pos_x = display_string("0", pos_x, 2);
    pos_x = display_string(hours, pos_x, 2);
    pos_x = display_string(":", pos_x, 2);
    if (min < 10)
        pos_x = display_string("0", pos_x, 2);
    pos_x = display_string(minutes, pos_x, 2);
    pos_x = display_string("]", pos_x, 2);
}

void Launcher::run()
{
    is_running = true;
    struct pollfd polls[1];
    polls[0].events = POLLIN;
    polls[0].fd = window_events_file;

    while (1) {
        display_time();
        request_update_window();
        // poll(polls, 1);
        sleep(5);
        receive_events();
        request_update_window();

        if (!is_running)
            return;
    }
}
