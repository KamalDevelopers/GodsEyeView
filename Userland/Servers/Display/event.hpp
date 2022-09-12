#ifndef EVENT_HPP
#define EVENT_HPP

#include "wm.hpp"

typedef struct events_files {
    int mouse = 0;
    int keyboard = 0;
} events_files_t;

bool mouse_events(mouse_event_t* event);
void send_events(WindowManager* wm);
events_files_t init_events();

#endif
