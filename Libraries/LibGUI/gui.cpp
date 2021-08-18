#include "gui.hpp"

using namespace GUI;

Image::Image(int width, int height, short int* bmp)
{
    widget_width = width;
    widget_height = height;

    if (bmp != 0) {
        bitmap = new short int[width * height];
        memcpy((void*)bitmap, (void*)bmp, (width * height) * sizeof(short int));
    } else {
        memchr(image_data, '\0', width * height);
    }
}

uint16_t Image::get_color(int index)
{
    if (is_rendered == 0)
        return bitmap[index];
    return image_data[index];
}

void Image::image_renderer(unsigned char* data)
{
    int i = 0;
    for (int y = 0; y < widget_height; y++) {
        for (int x = 0; x < widget_width; x++) {
            // clang-format off
            switch (data[i]) {
            case '0': image_data[i] = 0; break;
            case '1': image_data[i] = 1; break;
            case '2': image_data[i] = 2; break;
            case '3': image_data[i] = 3; break;
            case '4': image_data[i] = 4; break;
            case '5': image_data[i] = 5; break;
            case '6': image_data[i] = 6; break;
            case '7': image_data[i] = 7; break;
            case '8': image_data[i] = 8; break;
            case '9': image_data[i] = 9; break;
            case 'A': image_data[i] = 10; break;
            case 'B': image_data[i] = 11; break;
            case 'C': image_data[i] = 12; break;
            case 'D': image_data[i] = 13; break;
            case 'E': image_data[i] = 14; break;
            case 'F': image_data[i] = 15; break;
            }
            // clang-format on
            i++;
        }
    }
    is_rendered = 1;
    if (bitmap != 0)
        kfree(bitmap);
}

void Image::add(Graphics* vga, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height)
{
    int index = 0;
    for (int y = 0; y < widget_height; y++) {
        for (int x = 0; x < widget_width; x++) {
            if ((x > parent_width) || (y > parent_height))
                return;

            if (bitmap[index] != -1)
                vga->put_pixel(x + parent_pos_x, y + parent_pos_y, bitmap[index]);
            index++;
        }
    }
}

Label::Label(int xpos, int ypos, int width, int height, uint8_t fcolor, uint8_t bcolor, char* text)
{
    widget_xpos = xpos;
    widget_ypos = ypos;
    widget_width = width;
    widget_height = height;

    widget_color = fcolor;
    box_color = bcolor;
    widget_text = text;
}

void Label::add(Graphics* vga, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height)
{
    int xsize = str_len(widget_text) * 8; //8 = Bitmap size
    int twidget_xpos = widget_xpos + parent_pos_x;
    int twidget_ypos = widget_ypos + parent_pos_y;

    if (xsize > parent_width)
        return;

    if (twidget_xpos + xsize > parent_width + parent_pos_x)
        return;

    if (twidget_ypos + 8 > parent_height + parent_pos_y)
        return;

    for (int y = 0; y < widget_height; y++) {
        for (int x = 0; x < widget_width + xsize; x++)
            vga->put_pixel(twidget_xpos + x, twidget_ypos + y, box_color);
    }

    vga->reset_offset();
    vga->print(widget_text, widget_color, twidget_xpos, twidget_ypos);
}

Input::Input(int xpos, int ypos, int width, int height, uint8_t fcolor, uint8_t bcolor, char* text)
{
    widget_xpos = xpos;
    widget_ypos = ypos;
    widget_width = width;
    widget_height = height;

    widget_color = fcolor;
    box_color = bcolor;
    widget_text = text;
}

void Input::add(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height)
{
    int twidget_xpos = widget_xpos + parent_pos_x;
    int twidget_ypos = widget_ypos + parent_pos_y;

    for (int y = 0; y < widget_height; y++) {
        for (int x = 0; x < widget_width; x++) {
            int x_t = twidget_xpos + x;
            int y_t = twidget_ypos + y;
            int x_e = parent_pos_x + parent_width;
            int y_e = parent_pos_y + parent_height;

            if ((mouse->get_mouse_x() == x_t) && (mouse->get_mouse_y() == y_t) && (mouse->get_mouse_press() == 1))
                active_input = 1;
            if ((active_input != 1) && (hitbox_expand == 1) && (mouse->get_mouse_x() < parent_pos_x + parent_width) && (mouse->get_mouse_x() > parent_pos_x))
                if ((mouse->get_mouse_y() < parent_pos_y + parent_height) && (mouse->get_mouse_y() > parent_pos_y) && (mouse->get_mouse_press() == 1))
                    active_input = 1;

            if (input_text_index != keyboard->get_key_presses() - backslashoffset - enteroffset) {
                if (active_input == 1) {
                    if ((keyboard->get_last_key() == '\b') && (input_text_index != 0)) {
                        input_text[input_text_index - backslashoffset - 1] = ' ';
                        backslashoffset++;
                    }
                    if ((keyboard->get_last_key() != '\n') && (keyboard->get_last_key() != '\b')) {
                        if (((input_text_index * 8) + (str_len(widget_text) * 8)) < widget_width) {
                            input_text[input_text_index - backslashoffset] = keyboard->get_last_key();
                            input_text[input_text_index - backslashoffset + 1] = '\0';
                            input_text_index++;
                        }
                    }
                    if (keyboard->get_last_key() == '\n') {
                        strcpy(out_data, input_text);
                        //on_press(out_data);

                        for (int i = 0; i < input_text_index; i++)
                            input_text[i] = '\0';
                        enteroffset = enteroffset + (input_text_index + 1);

                        vga->reset_offset();
                        vga->print(widget_text, widget_color, twidget_xpos, twidget_ypos);
                        input_text_index = 0;
                    }
                } else {
                    enteroffset++;
                }
                vga->put_pixel(x_t, y_t, box_color);
            }
        }
    }
    vga->reset_offset();
    vga->print(widget_text, widget_color, twidget_xpos, twidget_ypos);
    vga->print(input_text, widget_color, twidget_xpos, twidget_ypos);
}

Button::Button(int xpos, int ypos, int width, int height, char* text, void (*op)(void))
{
    widget_xpos = xpos;
    widget_ypos = ypos;
    widget_width = width;
    widget_height = height;

    widget_text = text;
    on_press = op;
}

void Button::add(Graphics* vga, MouseDriver* mouse, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height)
{
    int xsize = str_len(widget_text) * 8; //8 = Bitmap size
    int twidget_xpos = widget_xpos + parent_pos_x;
    int twidget_ypos = widget_ypos + parent_pos_y;

    if (xsize > parent_width)
        return;

    if (twidget_xpos + xsize > parent_width + parent_pos_x)
        return;

    if (twidget_ypos + 8 > parent_height + parent_pos_y)
        return;

    for (int y = 0; y < widget_height; y++)
        for (int x = 0; x < widget_width + xsize; x++) {
            if ((on_hover_state == 2) && (is_mouse))
                vga->put_pixel(twidget_xpos + x + 1, twidget_ypos + y + 1, shadow_color);
            else
                vga->put_pixel(twidget_xpos + x + shadow_offset, twidget_ypos + y + shadow_offset, shadow_color);
        }

    on_hover_state = 0;
    for (int y = 0; y < widget_height; y++) {
        for (int x = 0; x < widget_width + xsize; x++) {
            if ((mouse->get_mouse_x() == twidget_xpos + x) && (mouse->get_mouse_y() == twidget_ypos + y) && (shadow_offset != 0)) {
                if (mouse->get_mouse_press() == 1)
                    on_hover_state = 2;
                else
                    on_hover_state = 1;
            }
        }
    }

    if (!is_mouse)
        on_hover_state = 0;

    for (int y = 0; y < widget_height; y++) {
        for (int x = 0; x < widget_width + xsize; x++) {
            int x_t = twidget_xpos + x;
            int y_t = twidget_ypos + y;

            if ((mouse->get_mouse_x() == x_t) && (mouse->get_mouse_y() == y_t))
                if ((mouse->get_mouse_press() == 1) && (is_mouse))
                    on_press();

            vga->put_pixel(x_t + on_hover_state, y_t + on_hover_state, box_color);
        }
    }

    vga->reset_offset();
    vga->print(widget_text, widget_color, on_hover_state + twidget_xpos + 1 + widget_width / 2, on_hover_state + twidget_ypos - 4 + widget_height / 2);
    if (render_image == 1)
        image->add(vga, on_hover_state + twidget_xpos - 5 + widget_width / 2, on_hover_state + twidget_ypos - 4 + widget_height / 2, widget_width, widget_height);
}

void Button::add_image(Image* img)
{
    image = img;
    render_image = 1;
}

Panel::Panel(int xpos, int ypos, int width, int height, uint8_t color)
{
    widget_xpos = xpos;
    widget_ypos = ypos;
    widget_width = width;
    widget_height = height;
    panel_color = color;
}

void Panel::add(Graphics* vga, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height)
{
    int twidget_xpos = widget_xpos + parent_pos_x;
    int twidget_ypos = widget_ypos + parent_pos_y;

    for (int y = 0; y < widget_height; y++)
        for (int x = 0; x < widget_width; x++) {
            int x_t = twidget_xpos + x;
            int y_t = twidget_ypos + y;

            vga->put_pixel(x_t, y_t, panel_color);
        }
}

ProgressBar::ProgressBar(int xpos, int ypos, int length)
{
    widget_xpos = xpos;
    widget_ypos = ypos;
    widget_length = length;
}

void ProgressBar::add(Graphics* vga, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height)
{
    int twidget_xpos = widget_xpos + parent_pos_x;
    int twidget_ypos = widget_ypos + parent_pos_y;

    for (int y = 0; y < widget_height + 2; y++) {
        for (int x = 0; x < widget_length + 2; x++) {
            int x_t = twidget_xpos + x - 1;
            int y_t = twidget_ypos + y - 1;

            vga->put_pixel(x_t, y_t, border_color);
        }
    }

    for (int y = 0; y < widget_height; y++) {
        for (int x = 0; x < widget_length; x++) {
            int x_t = twidget_xpos + x;
            int y_t = twidget_ypos + y;

            vga->put_pixel(x_t, y_t, widget_color);
        }
    }

    if (progress > 100)
        progress = 100;
    if (progress < 0)
        progress = 0;

    float progress_in_pixels = (progress / 100) * widget_length;

    for (int y = 0; y < widget_height; y++) {
        for (int x = 0; x < progress_in_pixels; x++) {
            int x_t = twidget_xpos + x;
            int y_t = twidget_ypos + y;

            vga->put_pixel(x_t, y_t, bar_color);
        }
    }
}

/* Not Finished */
CheckBox::CheckBox(int xpos, int ypos)
{
    widget_xpos = xpos;
    widget_ypos = ypos;
}

/* Not Finished */
void CheckBox::add(Graphics* vga, MouseDriver* mouse, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height)
{
    int twidget_xpos = widget_xpos + parent_pos_x;
    int twidget_ypos = widget_ypos + parent_pos_y;

    /* Fill */

    for (int y = 0; y < widget_height; y++)
        for (int x = 0; x < widget_width; x++) {
            int x_t = twidget_xpos + x;
            int y_t = twidget_ypos + y;
            if ((mouse->get_mouse_x() == x_t) && (mouse->get_mouse_y() == y_t)) {
                if ((state_locked == -1) && (mouse->get_mouse_press() == 1)) {
                    if (state == 1) {
                        state = 0;
                    } else {
                        state = 1;
                    }
                    state_locked = Time->get_second();
                }
                if (Time->get_second() != state_locked) {
                    state_locked = -1;
                }
            }
            if (state == 1)
                vga->put_pixel(x_t, y_t, active_color);
            if (state == 0)
                vga->put_pixel(x_t, y_t, normal_color);
        }
}

Slider::Slider(int xpos, int ypos, int width)
{
    widget_xpos = xpos;
    widget_ypos = ypos;
    widget_width = width;
}

void Slider::add(Graphics* vga, MouseDriver* mouse, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height)
{
    int twidget_xpos = widget_xpos + parent_pos_x;
    int twidget_ypos = widget_ypos + parent_pos_y;

    for (int y = 0; y < slider_height; y++) {
        for (int x = 0; x < widget_width + knob_width; x++) {
            int x_t = twidget_xpos + x;
            int y_t = twidget_ypos + y;

            vga->put_pixel(x_t, y_t, slider_color);
            vga->put_pixel(x_t + 1, y_t + 1, 0x0);
        }
    }

    for (int y = 0; y < knob_height; y++) {
        for (int x = 0; x < knob_width; x++) {
            int x_t = twidget_xpos + x;
            int y_t = twidget_ypos + y - knob_height / 3;

            vga->put_pixel(x_t + value_in_pixels, y_t, knob_color);
            vga->put_pixel(x_t + 1 + value_in_pixels, y_t + 1, 0x0);
        }
    }

    for (int y = 0; y < knob_height; y++) {
        for (int x = 0; x < widget_width + 2; x++) {
            int x_t = twidget_xpos - 1 + x;
            int y_t = twidget_ypos + y - knob_height / 3;

            if ((mouse->get_mouse_x() == x_t) && (mouse->get_mouse_y() == y_t))
                if (mouse->get_mouse_press() == 1)
                    value_in_pixels = mouse->get_mouse_x() - twidget_xpos;
        }
    }
}

Window::Window(int xpos, int ypos, int w, int h, uint8_t color, uint8_t wb)
{
    win_bar = wb;
    win_xpos = xpos;
    win_ypos = ypos;
    win_height = h;
    win_width = w;
    win_color = color;
}

void Window::begin_children(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard)
{
    if (save_mouse_press == 1)
        return;

    for (int i = 0; i < widget_indexLabel; i++)
        childrenLabel[i]->add(vga, win_xpos, win_ypos, win_width, win_height);

    for (int i = 0; i < widget_indexButton; i++)
        childrenButton[i]->add(vga, mouse, win_xpos, win_ypos, win_width, win_height);

    for (int i = 0; i < widget_indexInput; i++)
        childrenInput[i]->add(vga, mouse, keyboard, win_xpos, win_ypos, win_width, win_height);

    for (int i = 0; i < widget_indexPanel; i++)
        childrenPanel[i]->add(vga, win_xpos, win_ypos, win_width, win_height);

    for (int i = 0; i < widget_indexProgressBar; i++)
        childrenProgressBar[i]->add(vga, win_xpos, win_ypos, win_width, win_height);

    for (int i = 0; i < widget_indexCheckBox; i++)
        childrenCheckBox[i]->add(vga, mouse, win_xpos, win_ypos, win_width, win_height);

    for (int i = 0; i < widget_indexSlider; i++)
        childrenSlider[i]->add(vga, mouse, win_xpos, win_ypos, win_width, win_height);
}

uint8_t Window::begin(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard)
{
    for (int y = 0 + win_ypos; y < win_height + win_ypos; y++) {
        for (int x = 0 + win_xpos; x < win_width + win_xpos; x++) {
            if (save_mouse_press != 1)
                vga->put_pixel(x, y, win_color);
            else
                break;

            if (x == win_xpos)
                for (int i = 0; i < border_thickness; i++)
                    vga->put_pixel(x + i, y, border_color);

            if (x == win_xpos + win_width - 1)
                for (int i = 0; i < border_thickness; i++)
                    vga->put_pixel(x - i, y, border_color);

            if (y == win_ypos)
                for (int i = 0; i < border_thickness; i++)
                    if (win_bar != 1)
                        vga->put_pixel(x, y + i, border_color);

            if (y == win_ypos + win_height - 1)
                for (int i = 0; i < border_thickness; i++)
                    vga->put_pixel(x, y - i, border_color);
        }
    }

    if (win_bar != 1) {
        begin_children(vga, mouse, keyboard);
        return 1;
    }

    for (int y = win_ypos; y < win_ypos + 10; y++) {
        for (int x = win_xpos; x < win_width + win_xpos; x++) {
            if ((mouse->get_mouse_x() == x) && (mouse->get_mouse_y() == y - 4) && (mouse->get_mouse_press() == 1)) {
                if (x < win_width + win_xpos - 10) {
                    mouse_offset_x = x - win_xpos;
                    save_mouse_press = 1;
                    ms_disable();
                }
            }

            if ((mouse->get_mouse_press() == 0) && (save_mouse_press == 1)) {
                vga->fill_rectangle(win_xpos + 1, win_ypos + 8, win_width - 2, win_height - 9, win_color);
                save_mouse_press = 0;
                ms_enable();
            }

            if (save_mouse_press == 1) {
                win_xpos = mouse->get_mouse_x() - mouse_offset_x;
                win_ypos = mouse->get_mouse_y();
                if (win_xpos + win_width >= vga->get_screen_w())
                    win_xpos = vga->get_screen_w() - win_width;
                if (win_xpos <= 1)
                    win_xpos = 1;
                if (win_ypos + win_height >= vga->get_screen_h() - 10)
                    win_ypos = vga->get_screen_h() - 10 - win_height;
                if (win_ypos <= 25)
                    win_ypos = 25;
            }

            if ((save_mouse_press != 1) || ((y - 4 == win_ypos) && (x > 3 + win_xpos + str_len(win_title) * 8) && (x < win_xpos + win_width - 10)))
                vga->put_pixel(x, y - 4, 0x7);
        }
    }

    int index = 0;
    for (int x = 0 + win_xpos + win_width - 10; x < 10 + win_xpos + win_width - 10; x++) {
        for (int y = 0 + win_ypos - 4; y < 10 + win_ypos - 4; y++) {
            if ((mouse->get_mouse_x() == x) && (mouse->get_mouse_y() == y - 4) && (mouse->get_mouse_press() == 1) && (is_mouse)) {
                win_hidden = 1;
                save_mouse_press = 0;
                ms_enable();
            }

            if (closewindow_bitmap[index] != -1)
                vga->put_pixel(x, y, closewindow_bitmap[index]);
            index++;
        }
    }

    vga->reset_offset();
    vga->print(win_title, 0x0, win_xpos + 5, win_ypos - 2);
    begin_children(vga, mouse, keyboard);
    return 1;
}

void Window::border(uint8_t thickness, uint8_t color)
{
    border_thickness = thickness;
    border_color = color;
}

void Window::mouse_release(uint32_t x, uint32_t y, int b)
{
    mouse_down = 0;
}

void Window::mouse_press(uint32_t x, uint32_t y, int b, Graphics* vga)
{
    mouse_down = 1;
    if (win_bar == 1) {
        for (int bar_y = 0 + win_ypos; bar_y < 4 + win_ypos; bar_y++) {
            for (int bar_x = 0 + win_xpos; bar_x < win_width + win_xpos; bar_x++) {
                if ((bar_x == x) and (bar_y == y)) {
                    win_xpos = x;
                    win_ypos = y;
                }
            }
        }
    }
}

Desktop::Desktop(int w, int h, Graphics* g, MouseDriver* m, KeyboardDriver* k)
{
    desk_height = h;
    desk_width = w;
    vga = g;
    mouse = m;
    keyboard = k;
}

void Desktop::mouse_release(uint32_t x, uint32_t y, int b)
{
    for (int i = 0; i < win_index; i++)
        children[i]->mouse_release(x, y, b);
}

void Desktop::mouse_press(uint32_t x, uint32_t y, int b)
{
    for (int i = 0; i < win_index; i++)
        children[i]->mouse_press(x, y, b, vga);
}

void Desktop::set_wallpaper(Image* img)
{
    desk_wallpaper = img;
    for (int y = 0; y < 480; y++)
        for (int x = 0; x < 640; x++)
            vga->set_background(x, y, desk_wallpaper->get_color(y * 640 + x));
}

int Desktop::add_win(int count, ...)
{
    if (count > 100)
        return 1;

    va_list list;
    int j = 0;

    va_start(list, count);
    for (j = 0; j < count; j++) {
        children[win_index] = va_arg(list, Window*);
        win_index++;
    }

    va_end(list);
    return 0;
}

int Desktop::append_win(Window* win)
{
    win_index++;
    children[win_index] = win;
    return 0;
}

void Desktop::draw()
{
    auto mouse_disable = []() { is_mouse = 0; };
    auto mouse_enable = []() { is_mouse = 1; };

    for (int i = 0; i < win_index; i++) {
        if (children[i]->get_destroy() == 1) {
            delete_element(i, win_index, children);
            win_index = win_index - 1;
        } else if (children[i]->get_hidden() != 1)
            children[i]->begin(vga, mouse, keyboard);
    }

    vga->render_mouse(mouse_bitmap, mouse->get_mouse_x(), mouse->get_mouse_y());
    vga->render_screen();
}

void Desktop::start()
{
    while (1)
        draw();
}
