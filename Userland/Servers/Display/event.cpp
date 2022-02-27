#include "event.hpp"

events_files_t events_files;
bool mouse_events(mouse_event_t* event)
{
    if (read(events_files.mouse, (void*)event, sizeof(mouse_event_t)))
        return true;
    return false;
}

bool keyboard_events(keyboard_event_t* event)
{
    if (read(events_files.keyboard, (void*)event, sizeof(keyboard_event_t)))
        return true;
    return false;
}

void send_events(WindowManager* wm)
{
    mouse_event_t mouse_event;
    if (mouse_events(&mouse_event))
        wm->mouse_event(&mouse_event);

    keyboard_event_t keyboard_event;
    if (keyboard_events(&keyboard_event))
        wm->keyboard_event(&keyboard_event);
}

events_files_t init_events()
{
    events_files.mouse = open((char*)"/dev/mouse");
    events_files.keyboard = open((char*)"/dev/keyboard");
    return events_files;
}
