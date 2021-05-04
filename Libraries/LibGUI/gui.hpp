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

    void Add(Graphics* vga, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    void ImageRenderer(unsigned char* data);
    uint16_t GetColor(int index);
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
    void Add(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    int GetType() { return 4; }
    void HitboxExpand(int value) { hitbox_expand = value; }
    void SetListener(void (*op)(char*)) { on_press = op; }
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
    void Add(Graphics* vga, MouseDriver* mouse, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    void AddImage(Image* img);
    void Color(uint8_t color) { box_color = color; }
    void ShadowColor(uint8_t scolor) { shadow_color = scolor; }
    void ShadowOffset(int soffset) { shadow_offset = soffset; }
    int GetType() { return 3; }
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
    void Add(Graphics* vga, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    int GetType() { return 2; }
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
    void Add(Graphics* vga, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    void SetText(char* new_text) { widget_text = new_text; }
    int GetType() { return 1; }
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
    void Add(Graphics* vga, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    void SetProgress(float percentage) { progress = percentage; }
    float GetProgress() { return progress; }
    void Color(uint8_t c) { widget_color = c; }
    void BorderColor(uint8_t c) { border_color = c; }
    void BarColor(uint8_t c) { bar_color = c; }
    void TextColor(uint8_t c) { text_color = c; }
    void ShowText(uint8_t s) { show_text = s; }
    int GetType() { return 5; }
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
    void Add(Graphics* vga, MouseDriver* mouse, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    void BorderColor(uint8_t c) { border_color = c; }
    void NormalColor(uint8_t c) { normal_color = c; }
    void ActiveColor(uint8_t c) { active_color = c; }
    int GetType() { return 6; }
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
    int valueInPixels;

public:
    Slider(int xpos, int ypos, int width);
    void Add(Graphics* vga, MouseDriver* mouse, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    void SetValue(float v) { value = v; }
    float GetValue() { return value; }
    int GetType() { return 7; }
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
    uint8_t Begin(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard);
    void BeginChildren(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard);

    template<class T>
    void AddWidget(T* data)
    {
        switch (data->GetType()) {
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

    void Border(uint8_t thickness, uint8_t color);
    void SetTitle(char* wt) { win_title = wt; }

    void MousePress(uint32_t x, uint32_t y, int b, Graphics* vga);
    void MouseRelease(uint32_t x, uint32_t y, int b);

    void SetHidden(uint8_t hide) { win_hidden = hide; }

    void Revive() { destroy_win = 0; }
    void Destroy() { destroy_win = 1; }

    int GetDestroy() { return destroy_win; }
    int GetHidden() { return win_hidden; }
    int GetWidth() { return win_width; }
    int GetHeight() { return win_height; }
    int GetPosX() { return win_xpos; }
    int GetPosY() { return win_ypos; }
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
    void Start();
    void Draw();
    int AddWin(int count, ...);
    int AppendWin(Window* win);
    void SetWallpaper(Image* img);
    void WinDestroy(int index) { children[index] = 0; }

    void MouseRelease(uint32_t MouseX, uint32_t MouseY, int b);
    void MousePress(uint32_t MouseX, uint32_t MouseY, int b);
    void DrawMouse(int32_t x, int32_t y);

    Graphics* GetVGA() { return vga; }
    MouseDriver* GetMouse() { return mouse; };
    KeyboardDriver* GetKeyboard() { return keyboard; };
};
};

#endif
