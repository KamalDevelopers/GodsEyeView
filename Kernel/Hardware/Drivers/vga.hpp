#ifndef VGA_HPP
#define VGA_HPP

#include "../../Mem/mm.hpp"
#include "../../tty.hpp"
#include "../port.hpp"
#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "LibC/string.hpp"
#include "LibC/types.hpp"

namespace VGA16 {
static uint8_t black = 0x0;
static uint8_t blue = 0x1;
static uint8_t green = 0x2;
static uint8_t cyan = 0x3;
static uint8_t red = 0x4;
static uint8_t magenta = 0x5;
static uint8_t brown = 0x6;
static uint8_t white = 0x7;
static uint8_t gray = 0x8;
static uint8_t light_blue = 0x9;
static uint8_t light_green = 0xA;
static uint8_t light_cyan = 0xB;
static uint8_t light_red = 0xC;
static uint8_t light_magenta = 0xD;
static uint8_t yellow = 0xE;
static uint8_t bright_white = 0xF;
};

static uint8_t vga_on = 0;

class Graphics {
protected:
    Port8Bit miscPort;
    Port8Bit crtcIndexPort;
    Port8Bit crtcDataPort;
    Port16Bit sequencerIndexPort;
    Port8Bit sequencerDataPort;
    Port16Bit graphicsControllerIndexPort;
    Port8Bit graphicsControllerDataPort;
    Port8Bit attributeControllerIndexPort;
    Port8Bit attributeControllerReadPort;
    Port8Bit attributeControllerWritePort;
    Port8Bit attributeControllerResetPort;

    void write_registers(uint8_t* registers);
    uint8_t* get_frame_buffer_segment();
    void vga_draw(uint32_t x, uint32_t y, uint8_t colorindex, int cycle);
    void slow_draw(uint32_t x, uint32_t y, uint8_t color_index);

    uint8_t vga_buffer[480 * 640];
    uint8_t old_vga_buffer[480 * 640];
    uint8_t background[480 * 640];

    uint8_t is_ready = 0;

private:
    int vga_x_offset = 0;
    int vga_y_offset = 0;
    int screen_width = 0;
    int screen_height = 0;
    int screen_colordepth = 0;

public:
    Graphics();
    ~Graphics();

    bool init(uint32_t width, uint32_t height, uint32_t colordepth, uint8_t colorindex);
    bool set_mode(uint32_t width, uint32_t height, uint32_t colordepth);

    void fill_rectangle(int x, int y, int wd, int ht, uint8_t colorindex);
    void fill_plane(int x, int y, int wd, int ht, unsigned c);

    void set_background(int x, int y, uint8_t colorindex);
    void render_screen(uint8_t refresh = 0);
    uint8_t* get_pixel_color(int x, int y);
    void render_mouse(short int bitmap[], int mx, int my);
    void put_pixel(uint32_t x, uint32_t y, uint8_t colorindex);
    void render_bit_map(int bitmap[], uint8_t colorindex, int x_offset = 0, int y_offset = 0);
    void print(char* str, uint8_t colorindex, int x_offset = 0, int y_offset = 0);
    void reset_offset();
    void decrease_offset(int x) { vga_x_offset -= x; }
    void set_plane(unsigned p);
    int get_screen_h() { return screen_height; }
    int get_screen_w() { return screen_width; }
    int get_screen_c() { return screen_colordepth; }
    void frame_start(uint8_t i) { is_ready = i; }
    uint8_t get_frame_start() { return is_ready; }
};

static void vmemset(unsigned char* s, unsigned c, unsigned n)
{
    for (; n != 0; n--) {
        *s = c;
        s++;
    }
}

#endif
