#ifndef GUI_HPP
#define GUI_HPP
#include "../../Kernel/Hardware/Drivers/cmos.hpp"
#include "../../Kernel/Hardware/Drivers/keyboard.hpp"
#include "../../Kernel/Hardware/Drivers/mouse.hpp"
#include "../../Kernel/Hardware/Drivers/vga.hpp"
#include "../../Kernel/Mem/mm.hpp"
#include "../LibC/stdlib.hpp"
#include "../LibC/string.hpp"
#include <stdarg.h>

namespace GUI {

class Image {
private:
    int widget_width;
    int widget_height;
    short int* bitmap;
    uint8_t image_data[640 * 480];
    bool is_rendered = 0;

public:
    Image(int width, int height, short int* bmp);
    ~Image()
    {
        if ((is_rendered == 0) && (bitmap != 0))
            kfree(bitmap);
    }

    void add(Graphics* vga, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height);
    void image_renderer(unsigned char* data);
    uint16_t get_color(int index);
};

class Input {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width;
    int widget_height;
    void (*on_press)(char*);

    int input_text_index = 0;
    int backslashoffset = 0;
    int enteroffset = 0;

    uint8_t active_input = 0;
    uint8_t widget_color;
    uint8_t box_color;
    uint8_t hitbox_expand = 0;
    char* input_text = " ";
    char* out_data = " ";

    char* widget_text;

public:
    Input(int xpos, int ypos, int width, int height, uint8_t fcolor, uint8_t bcolor, char* text);
    void add(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height);
    int get_type() { return 4; }
    void set_hitbox_expand(int value) { hitbox_expand = value; }
    void set_listener(void (*op)(char*)) { on_press = op; }
};

class Button {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width;
    int widget_height;
    int shadow_offset = 2;
    uint8_t on_hover_state = 0;
    uint8_t widget_color = 0x0;
    uint8_t box_color = 0xF;
    uint8_t shadow_color = 0x0;
    uint8_t render_image = 0;
    char* widget_text;
    MouseDriver* mouse;
    Image* image;
    void (*on_press)(void);

public:
    Button(int xpos, int ypos, int width, int height, char* text, void (*op)(void));
    void add(Graphics* vga, MouseDriver* mouse, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height);
    void add_image(Image* img);
    void color(uint8_t color) { box_color = color; }
    void set_shadow_color(uint8_t scolor) { shadow_color = scolor; }
    void set_shadow_offset(int soffset) { shadow_offset = soffset; }
    int get_type() { return 3; }
};

class Panel {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width;
    int widget_height;
    uint8_t panel_color;

public:
    Panel(int xpos, int ypos, int width, int height, uint8_t color);
    void add(Graphics* vga, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height);
    int get_type() { return 2; }
};

class Label {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width;
    int widget_height;
    uint8_t widget_color;
    uint8_t box_color;
    char* widget_text;

public:
    Label(int xpos, int ypos, int width, int height, uint8_t fcolor, uint8_t bcolor, char* text);
    void add(Graphics* vga, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height);
    void set_text(char* new_text) { widget_text = new_text; }
    int get_type() { return 1; }
};

class ProgressBar {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_length;
    int widget_height = 20;
    uint8_t widget_color = 0x8;
    uint8_t border_color = 0x7;
    uint8_t bar_color = 0x2;
    uint8_t text_color = 0xF;

    uint8_t show_text = 1;
    float progress = 0;

public:
    ProgressBar(int xpos, int ypos, int length);
    void add(Graphics* vga, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height);
    void set_progress(float percentage) { progress = percentage; }
    float get_progress() { return progress; }
    void color(uint8_t c) { widget_color = c; }
    void set_border_color(uint8_t c) { border_color = c; }
    void set_bar_color(uint8_t c) { bar_color = c; }
    void set_text_color(uint8_t c) { text_color = c; }
    void set_show_text(uint8_t s) { show_text = s; }
    int get_type() { return 5; }
};

class CheckBox {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width = 14;
    int widget_height = 14;
    int state_locked = -1;

    uint8_t border_color = 0x7;
    uint8_t normal_color = 0x8;
    uint8_t active_color = 0xF;
    uint8_t fill_color;

public:
    uint8_t state = 0;
    CheckBox(int xpos, int ypos);
    void add(Graphics* vga, MouseDriver* mouse, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height);
    void set_border_color(uint8_t c) { border_color = c; }
    void set_normal_color(uint8_t c) { normal_color = c; }
    void set_active_color(uint8_t c) { active_color = c; }
    int get_type() { return 6; }
};

class Slider {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width;
    int mouse_offset_x;

    int knob_width = 8;
    int knob_height = 16;
    uint8_t knob_color = 0x7;

    int slider_height = 5;
    uint8_t slider_color = 0x8;

    float value = 40;
    int value_in_pixels;

public:
    Slider(int xpos, int ypos, int width);
    void add(Graphics* vga, MouseDriver* mouse, int parent_pos_x, int parent_pos_y, int parent_width, int parent_height);
    void set_value(float v) { value = v; }
    float get_value() { return value; }
    int get_type() { return 7; }
};

class Window {
private:
    uint8_t destroy_win = 0;
    uint8_t win_hidden = 0;
    uint8_t save_mouse_press = 0;

    int mouse_down = 0;
    int mouse_offset_x;
    int widget_indexLabel;
    int widget_indexButton;
    int widget_indexInput;
    int widget_indexPanel;
    int widget_indexProgressBar;
    int widget_indexCheckBox;
    int widget_indexSlider;
    int win_height;
    int win_width;
    int win_xpos;
    int win_ypos;

    uint8_t win_color;
    uint8_t win_bar;
    uint8_t border_thickness = 1;
    uint8_t border_color = 0x8;
    char* win_title = " ";

    Label* childrenLabel[100];
    Button* childrenButton[100];
    Input* childrenInput[100];
    Panel* childrenPanel[100];
    ProgressBar* childrenProgressBar[100];
    CheckBox* childrenCheckBox[100];
    Slider* childrenSlider[100];

public:
    Window(int xpos, int ypos, int w, int h, uint8_t color, uint8_t win_bar = 1);
    uint8_t begin(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard);
    void begin_children(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard);

    template<class T>
    void add_widget(T* data)
    {
        switch (data->get_type()) {
        case 1:
            childrenLabel[widget_indexLabel] = (Label*)data;
            widget_indexLabel++;
            break;
        case 2:
            childrenPanel[widget_indexPanel] = (Panel*)data;
            widget_indexPanel++;
            break;
        case 3:
            childrenButton[widget_indexButton] = (Button*)data;
            widget_indexButton++;
            break;
        case 4:
            childrenInput[widget_indexInput] = (Input*)data;
            widget_indexInput++;
            break;
        case 5:
            childrenProgressBar[widget_indexProgressBar] = (ProgressBar*)data;
            widget_indexProgressBar++;
            break;
        case 6:
            childrenCheckBox[widget_indexCheckBox] = (CheckBox*)data;
            widget_indexCheckBox++;
            break;
        case 7:
            childrenSlider[widget_indexSlider] = (Slider*)data;
            widget_indexSlider++;
            break;
        }
    }

    void border(uint8_t thickness, uint8_t color);
    void set_title(char* wt) { win_title = wt; }

    void mouse_press(uint32_t x, uint32_t y, int b, Graphics* vga);
    void mouse_release(uint32_t x, uint32_t y, int b);

    void set_hidden(uint8_t hide) { win_hidden = hide; }

    void revive() { destroy_win = 0; }
    void destroy() { destroy_win = 1; }

    int get_destroy() { return destroy_win; }
    int get_hidden() { return win_hidden; }
    int get_width() { return win_width; }
    int get_height() { return win_height; }
    int get_pos_x() { return win_xpos; }
    int get_pos_y() { return win_ypos; }
};

static bool is_mouse = 1;
static void ms_enable()
{
    is_mouse = 1;
}
static void ms_disable()
{
    is_mouse = 0;
}

class Desktop {
private:
    int win_index;
    int desk_height;
    int desk_width;
    int old_mouse_x = 0;
    int old_mouse_y = 0;
    uint8_t old_mouse_color = 0;
    uint8_t active_window = 0;

    Image* desk_wallpaper;
    Window* children[100];
    Graphics* vga;
    MouseDriver* mouse;
    KeyboardDriver* keyboard;

public:
    Desktop(int w, int h, Graphics* g, MouseDriver* m, KeyboardDriver* k);
    void start();
    void draw();
    int add_win(int count, ...);
    int append_win(Window* win);
    void set_wallpaper(Image* img);
    void win_destroy(int index) { children[index] = 0; }

    void mouse_release(uint32_t mouse_x, uint32_t mouse_y, int b);
    void mouse_press(uint32_t mouse_x, uint32_t mouse_y, int b);

    Graphics* get_vga() { return vga; }
    MouseDriver* get_mouse() { return mouse; };
    KeyboardDriver* get_keyboard() { return keyboard; };
};
};

#endif
