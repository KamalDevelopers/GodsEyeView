#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <LibC/types.hpp>

typedef struct mouse_event {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t modifier = 0;
} mouse_event_t;

typedef struct keyboard_event {
    char key = 0;
    uint32_t modifier = 0;
} keyboard_event_t;

#endif
