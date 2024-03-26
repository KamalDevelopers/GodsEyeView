#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <LibC/types.h>

#define KEY_DOWN 1
#define KEY_UP 0

#define GLOBAL_WM_EVENT_TYPE_PROBE (1 << 0)
#define GLOBAL_WM_EVENT_TYPE_WORKSPACE (1 << 1)
#define GLOBAL_WM_EVENT_TYPE_DESTROY (1 << 2)

typedef struct global_wm_event {
    uint8_t type;
    uint16_t d0;
    uint16_t d1;
} global_wm_event_t;

typedef struct mouse_event {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t modifier = 0;
} mouse_event_t;

typedef struct keyboard_event {
    char key = 0;
    uint32_t modifier = 0;
    uint8_t state = 0;
} keyboard_event_t;

#endif
