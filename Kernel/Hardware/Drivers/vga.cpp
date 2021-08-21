#include "vga.hpp"

Graphics::Graphics()
    : miscPort(0x3c2)
    , crtcIndexPort(0x3d4)
    , crtcDataPort(0x3d5)
    , sequencerIndexPort(0x3c4)
    , sequencerDataPort(0x3c5)
    , graphicsControllerIndexPort(0x3ce)
    , graphicsControllerDataPort(0x3cf)
    , attributeControllerIndexPort(0x3c0)
    , attributeControllerReadPort(0x3c1)
    , attributeControllerWritePort(0x3c0)
    , attributeControllerResetPort(0x3da)
{
}

void Graphics::write_registers(uint8_t* registers)
{
    miscPort.write(*(registers++));

    for (uint8_t i = 0; i < 5; i++) {
        sequencerIndexPort.write(i);
        sequencerDataPort.write(*(registers++));
    }

    crtcIndexPort.write(0x03);
    crtcDataPort.write(crtcDataPort.read() | 0x80);
    crtcIndexPort.write(0x11);
    crtcDataPort.write(crtcDataPort.read() & ~0x80);

    registers[0x03] = registers[0x03] | 0x80;
    registers[0x11] = registers[0x11] & ~0x80;

    for (uint8_t i = 0; i < 25; i++) {
        crtcIndexPort.write(i);
        crtcDataPort.write(*(registers++));
    }

    for (uint8_t i = 0; i < 9; i++) {
        graphicsControllerIndexPort.write(i);
        graphicsControllerDataPort.write(*(registers++));
    }

    for (uint8_t i = 0; i < 21; i++) {
        attributeControllerResetPort.read();
        attributeControllerIndexPort.write(i);
        attributeControllerWritePort.write(*(registers++));
    }

    attributeControllerResetPort.read();
    attributeControllerIndexPort.write(0x20);
}

bool Graphics::set_mode(uint32_t width, uint32_t height, uint32_t colordepth)
{
    vga_on = 1;
    // clang-format off
    unsigned char g_320x200x256[] = {
    /* MISC */
        0x63,
    /* SEQ */
        0x03, 0x01, 0x0F, 0x00, 0x0E,
    /* CRTC */
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF,
    /* GC */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
        0xFF,
    /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00, 0x00
    };

    unsigned char g_720x480x16[] =
    {
    /* MISC */
        0xE7,
    /* SEQ */
        0x03, 0x01, 0x08, 0x00, 0x06,
    /* CRTC */
        0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
        0x00, 0x40, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00,
        0xEA, 0x0C, 0xDF, 0x2D, 0x08, 0xE8, 0x05, 0xE3,
        0xFF,
    /* GC */
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
        0xFF,
    /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x01, 0x00, 0x0F, 0x00, 0x00,
    };

    unsigned char g_640x480x16[] = {
    /* MISC */
        0xE3,
    /* SEQ */
        0x03, 0x01, 0x08, 0x00, 0x06,
    /* CRTC */
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
        0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
        0xFF,
    /* GC */
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
        0xFF,
    /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x01, 0x00, 0x0F, 0x00, 0x00
    };
    // clang-format on

    screen_width = width;
    screen_height = height;
    screen_colordepth = colordepth;

    if ((width == 720) && (height = 480) && (colordepth == 16)) {
        write_registers(g_720x480x16);
        return true;
    }

    if ((width == 640) && (height = 480) && (colordepth == 16)) {
        write_registers(g_640x480x16);
        return true;
    }

    if ((width == 320) && (height = 200) && (colordepth == 256)) {
        write_registers(g_320x200x256);
        return true;
    }
    write_string("VGA Error: Mode not supported.");
    screen_colordepth = 0;
    return false;
}

bool Graphics::init(uint32_t width, uint32_t height, uint32_t colordepth, uint8_t colorindex)
{
    set_mode(width, height, colordepth);

    for (int32_t y = 0; y < height; y++)
        for (int32_t x = 0; x < width; x++)
            put_pixel(x, y, colorindex);
    render_screen(1);
    return true;
}

uint8_t* Graphics::get_frame_buffer_segment()
{
    graphicsControllerIndexPort.write(0x06);
    uint8_t segment_number = graphicsControllerDataPort.read() & (3 << 2);
    switch (segment_number) {
    default:
    case 0 << 2:
        return (uint8_t*)0x00000;
    case 1 << 2:
        return (uint8_t*)0xA0000;
    case 2 << 2:
        return (uint8_t*)0xB0000;
    case 3 << 2:
        return (uint8_t*)0xB8000;
    }
}

uint8_t* Graphics::get_pixel_color(int x, int y)
{
    uint8_t* pixel_address;
    //uint16_t wd_in_bytes = screen_width / 8;
    //uint16_t wd_x = x / 8;

    /*if (screen_colordepth == 16)
        pixel_address = 0xA0000 + wd_in_bytes * y + wd_x;
    else
        pixel_address = 0xA0000 + screen_width * y + x;*/
    pixel_address = (uint8_t*)0x00000 + screen_width * y + x;
    return pixel_address;
}

void Graphics::fill_plane(int x, int y, int wd, int ht, unsigned c)
{
    unsigned w, wd_in_bytes, off;
    unsigned char lmask, rmask;
    int x2, y2;

    unsigned wd_x = x / 8;
    uint8_t* raster = (uint8_t*)0xA0000;

    x2 = x + wd - 1;
    w = (x2 >> 3) - (x >> 3) + 1;
    lmask = 0x00FF >> (x & 7);
    rmask = 0xFF80 >> (x2 & 7);

    if (w == 1)
        lmask &= rmask;

    wd_in_bytes = 640 / 8;
    off = wd_in_bytes * y + x / 8;

    if (c) {
        for (y2 = y; y2 < y + ht; y2++) {
            raster[off] |= lmask;
            if (w > 2)
                vmemset(raster + off + 1, 0xFF, w - 2);
            if (w > 1)
                raster[off + w - 1] |= rmask;
            off += wd_in_bytes;
        }
    } else {
        lmask = ~lmask;
        rmask = ~rmask;
        for (y2 = y; y2 < y + ht; y2++) {
            raster[off] &= lmask;
            if (w > 2)
                vmemset(raster + off + 1, 0, w - 2);
            if (w > 1)
                raster[off + w - 1] &= rmask;
            off += wd_in_bytes;
        }
    }
}

void Graphics::set_plane(unsigned p)
{
    static unsigned curr_p = -1u;
    unsigned char pmask;

    p &= 3;
    if (p == curr_p)
        return;
    curr_p = p;
    pmask = 1 << p;

    graphicsControllerIndexPort.write((p << 8) | 4);
    sequencerIndexPort.write((pmask << 8) | 2);
}

void Graphics::vga_draw(uint32_t x, uint32_t y, uint8_t colorindex, int cycle)
{
    if (screen_colordepth == 16) {
        unsigned mask, p, pmask;
        unsigned wd_x = x / 8;
        unsigned wd_in_bytes = screen_width / 8;
        uint8_t* pixel_address = (uint8_t*)0xA0000 + wd_in_bytes * y + wd_x;

        x = (x & 7) * 1;
        mask = 0x80 >> x;
        pmask = 1;

        pmask <<= cycle;
        if (pmask & colorindex)
            *pixel_address = *pixel_address | mask;
        else
            *pixel_address = *pixel_address & ~mask;
    }
    if (screen_colordepth == 256) {
        uint8_t* pixel_address = (uint8_t*)0xA0000 + screen_width * y + x;
        *pixel_address = colorindex;
    }
}

void Graphics::slow_draw(uint32_t x, uint32_t y, uint8_t color_index)
{
    if (screen_colordepth == 16) {
        unsigned mask, p, pmask;
        unsigned wd_x = x / 8;
        unsigned wd_in_bytes = screen_width / 8;
        uint8_t* pixel_address = (uint8_t*)0xA0000 + wd_in_bytes * y + wd_x;

        x = (x & 7) * 1;
        mask = 0x80 >> x;
        pmask = 1;
        for (p = 0; p < 4; p++) {
            set_plane(p);
            if (pmask & color_index)
                *pixel_address = *pixel_address | mask;
            else
                *pixel_address = *pixel_address & ~mask;
            pmask <<= 1;
        }
    }
    if (screen_colordepth == 256) {
        uint8_t* pixel_address = (uint8_t*)0xA0000 + screen_width * y + x;
        *pixel_address = color_index;
    }
}

void Graphics::fill_rectangle(int x, int y, int wd, int ht, uint8_t colorindex)
{
    unsigned char p, pmask;

    pmask = 1;
    for (p = 0; p < 4; p++) {
        set_plane(p);
        fill_plane(x, y, wd, ht, colorindex & pmask);
        pmask <<= 1;
    }

    for (int a = y; a < ht; a++) {
        for (int b = x; b < wd; b++) {
            old_vga_buffer[a * 640 + b] = colorindex;
            vga_buffer[a * 640 + b] = colorindex;
        }
    }
}

void Graphics::put_pixel(uint32_t x, uint32_t y, uint8_t colorindex)
{
    vga_buffer[y * 640 + x] = colorindex;
}

void Graphics::set_background(int x, int y, uint8_t colorindex)
{
    background[y * 640 + x] = colorindex;
}

void Graphics::render_screen(uint8_t refresh)
{
    if (refresh == 1) {
        fill_rectangle(0, 0, 640, 480, VGA16::black);
        return;
    }

    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            if (vga_buffer[y * 640 + x] != old_vga_buffer[y * 640 + x]) {
                slow_draw(x, y, vga_buffer[y * 640 + x]);
            }
        }
    }

    memcpy(old_vga_buffer, vga_buffer, sizeof(uint8_t) * 640 * 480);
    memcpy(vga_buffer, background, sizeof(uint8_t) * 640 * 480);
}

void Graphics::reset_offset()
{
    vga_x_offset = 0;
}

void Graphics::render_bit_map(int bitmap[], uint8_t colorindex, int x_offset, int y_offset)
{
    int x, y;
    int set;
    int mask;

    for (x = 0; x < 8; x++) {
        for (y = 0; y < 8; y++) {
            set = bitmap[x] & 1 << y;
            if (set != 0) {
                old_vga_buffer[(y_offset + x) * 640 + (x_offset + vga_x_offset + y)] = colorindex;
                vga_buffer[(y_offset + x) * 640 + (x_offset + vga_x_offset + y)] = colorindex;
                slow_draw(x_offset + vga_x_offset + y, y_offset + x, colorindex);
            }
        }
    }
    vga_x_offset += 8;
}
