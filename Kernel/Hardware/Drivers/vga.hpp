#ifndef VGA_HPP
#define VGA_HPP

#include "../../../Libraries/LibGUI/font.hpp"
#include "../../Mem/mm.hpp"
#include "../../tty.hpp"
#include "../port.hpp"
#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "LibC/string.hpp"
#include "LibC/types.hpp"

namespace VGA16 {
static uint8_t BLACK = 0x0;
static uint8_t BLUE = 0x1;
static uint8_t GREEN = 0x2;
static uint8_t CYAN = 0x3;
static uint8_t RED = 0x4;
static uint8_t MAGENTA = 0x5;
static uint8_t BROWN = 0x6;
static uint8_t WHITE = 0x7;
static uint8_t GRAY = 0x8;
static uint8_t LIGHT_BLUE = 0x9;
static uint8_t LIGHT_GREEN = 0xA;
static uint8_t LIGHT_CYAN = 0xB;
static uint8_t LIGHT_RED = 0xC;
static uint8_t LIGHT_MAGENTA = 0xD;
static uint8_t YELLOW = 0xE;
static uint8_t BRIGHT_WHITE = 0xF;
};

static uint8_t vga_on = 0;

typedef struct RectangleRender {
    int x;
    int y;
    int wd;
    int ht;
    uint8_t c;
} rec_t;

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

    void WriteRegisters(uint8_t* registers);
    uint8_t* GetFrameBufferSegment();
    void VgaDraw(uint32_t x, uint32_t y, uint8_t colorindex, int cycle);

    virtual uint8_t GetColorIndex(uint8_t r, uint8_t g, uint8_t b);

    uint8_t vga_buffer[480][640];
    uint8_t old_vga_buffer[480][640];
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

    virtual bool Init(uint32_t width, uint32_t height, uint32_t colordepth, uint8_t colorindex);
    virtual bool SetMode(uint32_t width, uint32_t height, uint32_t colordepth);

    virtual void FillRectangle(int x, int y, int wd, int ht, uint8_t colorindex);
    virtual void FillPlane(int x, int y, int wd, int ht, unsigned c);

    virtual void RenderScreen(uint8_t i = 0);
    virtual void PutPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);
    virtual uint8_t* GetPixelColor(int x, int y);
    virtual void PutPixel(uint32_t x, uint32_t y, uint8_t colorindex);
    virtual void RenderBitMap(int bitmap[], uint8_t colorindex, int x_offset = 0, int y_offset = 0);
    virtual void Print(char* str, uint8_t colorindex, int x_offset = 0, int y_offset = 0);
    virtual void ResetOffset();
    virtual void DecreaseOffset(int x) { vga_x_offset -= x; }
    virtual void SetPlane(unsigned p);
    virtual int GetScreenH() { return screen_height; }
    virtual int GetScreenW() { return screen_width; }
    virtual int GetScreenC() { return screen_colordepth; }
    virtual void FrameStart(uint8_t i) { is_ready = i; }
    virtual uint8_t GetFrameStart() { return is_ready; }
};

static void vmemset(unsigned char *s, unsigned c, unsigned n)
{
	for(; n != 0; n--)
	{
		*s = c;
		s++;
	}
}

#endif
