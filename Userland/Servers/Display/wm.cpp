#include "wm.hpp"
#include <LibC/mem.h>
#include <LibC/stdlib.h>
#include <LibC/unistd.h>

WindowManager::WindowManager(Compositor* compositor)
{
    this->compositor = compositor;
}

WindowManager::~WindowManager()
{
}

int WindowManager::find_window_with_pid(int pid)
{
    for (uint32_t i = 0; i < windows.size(); i++)
        if (windows[i]->get_pid() == pid)
            return i;
    return -1;
}

void WindowManager::set_active_window(uint32_t index)
{
    uint32_t old_active_window = active_window;
    active_window = index;
    if (active_window != -1)
        update_window_border(active_window);
    if (old_active_window != -1)
        update_window_border(old_active_window);
    compositor->require_update();
}

void WindowManager::mouse_event(mouse_event_t* event)
{
    if (event->modifier == 1) {
        for (uint32_t i = 0; i < windows.size(); i++) {
            if (windows[i]->is_point_in_window(event->x, event->y) && windows[i]->get_controlled())
                set_active_window(i);
        }
    }

    if (active_window != -1)
        windows[active_window]->mouse_event(event);
    if (!event->modifier)
        compositor->update_mouse_position(event->x, event->y);
}

void WindowManager::keyboard_event(keyboard_event_t* event)
{
    if ((event->key == 't') && (event->modifier == 2)) {
        spawn("bin/terminal", 0);
        return;
    }

    if (active_window != -1)
        windows[active_window]->keyboard_event(event);
}

void WindowManager::require_update(int pid)
{
    int index = find_window_with_pid(pid);
    if (index == -1)
        return;

    update_window_border(index);
    compositor->require_update_canvas(windows.at(index)->get_canvas());
}

void WindowManager::update_window_border(uint32_t index)
{
    windows[index]->get_canvas()->border_decoration = (index == active_window) ? WINDOW_ACTIVE_BORDER_COLOR : WINDOW_BORDER_COLOR;
}

void WindowManager::update_window_positions()
{
    uint32_t position_x = WINDOW_GAP;
    uint32_t position_y = WINDOW_GAP;
    uint32_t windows_tile_vertical = 2;
    uint32_t tile_vertical_max = CLAMP(tiled_windows, 1, windows_tile_vertical);
    uint32_t tile_horizontal_max = (tiled_windows > windows_tile_vertical) ? tiled_windows - 1 : 1;
    uint32_t vertical_section = compositor->screen_width() / tile_vertical_max;
    uint32_t horizontal_section = compositor->screen_height() / tile_horizontal_max;

    uint32_t tiled_index = 0;
    for (uint32_t index = 0; index < windows.size(); index++) {
        if (!windows[index]->get_controlled())
            continue;

        uint32_t height = horizontal_section - WINDOW_GAP * 2;
        uint32_t width = vertical_section - WINDOW_GAP;

        if (tiled_index >= tile_vertical_max - 1)
            width -= WINDOW_GAP;
        if (tiled_index == 0)
            height = compositor->screen_height() - WINDOW_GAP * 2;
        if (tiled_index > windows_tile_vertical)
            height += WINDOW_GAP;

        if (tiled_index == 0) {
            height -= WINDOW_TOP_GAP;
            position_y += WINDOW_TOP_GAP;
        }
        if (tiled_index == 1) {
            height -= WINDOW_TOP_GAP;
        }

        windows[index]->resize(width, height);
        windows[index]->set_position(position_x, position_y);

        if (windows_tile_vertical > 1) {
            position_x += width + WINDOW_GAP;
            windows_tile_vertical--;
        } else {
            position_y += height + WINDOW_GAP;
        }

        tiled_index++;
    }

    if (!tiled_windows)
        compositor->require_update();
    compositor->require_update_next();
    /* compositor->require_update(); */
}

Window* WindowManager::compose_window(int pid)
{
    Window* window = new Window(pid);
    return window;
}

void WindowManager::create_window(uint32_t width, uint32_t height, int pid, uint32_t bg, uint8_t flags)
{
    if (windows.size() >= MAX_WINDOWS)
        return;

    Window* window = compose_window(pid);
    windows.append(window);
    if ((flags & DISPLAY_FLAG_DISOWNED) > 0) {
        window->resize(width, height);
        window->disown();
    } else {
        tiled_windows++;
    }
    update_window_positions();
    nice(2);

    canvas_set(window->get_canvas()->framebuffer, bg, window->get_canvas()->size);
    window->get_canvas()->border_decoration = 0;
    if (((bg >> 24) & 0x000000FF) > 0)
        canvas_create_alpha(window->get_canvas(), bg);

    compositor->add_render_layer(window->get_canvas());
    window->create_process_connection();
    if (window->get_controlled())
        set_active_window(windows.size() - 1);
    nice(-2);
}

void WindowManager::destroy_window(uint32_t index)
{
    compositor->remove_render_layer(windows[index]->get_canvas());
    if (windows[index]->get_controlled())
        tiled_windows--;
    delete windows[index];
    windows.remove_at(index);

    if (!tiled_windows) {
        active_window = -1;
        return;
    }

    if (active_window > 0)
        active_window--;

    uint32_t next = CLAMP(active_window, 0, tiled_windows - 1);
    uint32_t current = 0;
    for (uint32_t i = 0; i < windows.size(); i++) {
        if (!windows.at(i)->get_controlled())
            continue;
        if (current >= next) {
            active_window = i;
            break;
        }
        current++;
    }
}

void WindowManager::destroy_window_pid(int pid)
{
    int index = find_window_with_pid(pid);
    if (index == -1)
        return;

    destroy_window(index);
    update_window_positions();
}
