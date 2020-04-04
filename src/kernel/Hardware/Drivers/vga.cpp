#include "vga.hpp"

Graphics::Graphics() : 
    miscPort(0x3c2),
    crtcIndexPort(0x3d4),
    crtcDataPort(0x3d5),
    sequencerIndexPort(0x3c4),
    sequencerDataPort(0x3c5),
    graphicsControllerIndexPort(0x3ce),
    graphicsControllerDataPort(0x3cf),
    attributeControllerIndexPort(0x3c0),
    attributeControllerReadPort(0x3c1),
    attributeControllerWritePort(0x3c0),
    attributeControllerResetPort(0x3da)
{
}

void Graphics::WriteRegisters(uint8_t* registers)
{
    miscPort.Write(*(registers++));

    for(uint8_t i = 0; i < 5; i++)
    {
        sequencerIndexPort.Write(i);
        sequencerDataPort.Write(*(registers++));
    }

    crtcIndexPort.Write(0x03);
    crtcDataPort.Write(crtcDataPort.Read() | 0x80);
    crtcIndexPort.Write(0x11);
    crtcDataPort.Write(crtcDataPort.Read() & ~0x80);

    registers[0x03] = registers[0x03] | 0x80;
    registers[0x11] = registers[0x11] & ~0x80;

    for(uint8_t i = 0; i < 25; i++)
    {
        crtcIndexPort.Write(i);
        crtcDataPort.Write(*(registers++));
    }

    for(uint8_t i = 0; i < 9; i++)
    {
        graphicsControllerIndexPort.Write(i);
        graphicsControllerDataPort.Write(*(registers++));
    }

    for(uint8_t i = 0; i < 21; i++)
    {
        attributeControllerResetPort.Read();
        attributeControllerIndexPort.Write(i);
        attributeControllerWritePort.Write(*(registers++));
    }

    attributeControllerResetPort.Read();
    attributeControllerIndexPort.Write(0x20);
}

bool Graphics::SetMode(uint32_t width, uint32_t height, uint32_t colordepth)
{
    vga_on = 1;

    unsigned char g_320x200x256[] =
    {
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

    unsigned char g_640x480x16[] =
    {
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

    screen_width = width;
    screen_height = height;
    screen_colordepth = colordepth;

    if ((width == 640) && (height = 480) && (colordepth == 16)){
        WriteRegisters(g_640x480x16);
        return true;
    }

    if ((width == 320) && (height = 200) && (colordepth == 256)){
        WriteRegisters(g_320x200x256);
        return true;
    }
    printf("%s\n", "VGA Error: Mode not supported.");
    screen_width, screen_height, screen_colordepth = 0;
    return false;
}

bool Graphics::Init(uint32_t width, uint32_t height, uint32_t colordepth, uint8_t colorIndex)
{
    SetMode(width, height, colordepth);
    for(int32_t y = 0; y < height; y++)
        for(int32_t x = 0; x < width; x++)
            PutPixel(x, y, colorIndex);
    return true;
}

uint8_t* Graphics::GetFrameBufferSegment()
{
    graphicsControllerIndexPort.Write(0x06);
    uint8_t segmentNumber = graphicsControllerDataPort.Read() & (3<<2);
    switch(segmentNumber)
    {
        default:
        case 0<<2: return (uint8_t*)0x00000;
        case 1<<2: return (uint8_t*)0xA0000;
        case 2<<2: return (uint8_t*)0xB0000;
        case 3<<2: return (uint8_t*)0xB8000;
    }
}

uint8_t* Graphics::GetPixelColor(int x, int y)
{
    uint8_t* pixelAddress = GetFrameBufferSegment() + 320*y + x;
    return pixelAddress;
}

void Graphics::set_plane(unsigned p)
{
    unsigned char pmask;
    p &= 3;
    pmask = 1 << p;
    graphicsControllerIndexPort.Write(4);
    graphicsControllerDataPort.Write(p);
    sequencerIndexPort.Write(2);
    sequencerDataPort.Write(pmask);
}

void Graphics::PutPixel(uint32_t x, uint32_t y, uint8_t colorIndex)
{
    if ((screen_width == 640) && (screen_height = 480) && (screen_colordepth == 16)){
        unsigned wd_in_bytes, wd_x, mask, p, pmask;

        wd_in_bytes = 640 / 8;
        wd_x = x / 8;

        uint8_t* pixelAddress = GetFrameBufferSegment() + wd_in_bytes * y + wd_x;

        x = (x & 7) * 1;
        mask = 0x80 >> x;
        pmask = 1;
        for(p = 0; p < 4; p++)
        {
            set_plane(p);
            if(pmask & colorIndex)
                *pixelAddress = *pixelAddress | mask;
            else
                *pixelAddress = *pixelAddress & ~mask;
            pmask <<= 1;
        }
    }
    if ((screen_width == 320) && (screen_height = 200) && (screen_colordepth == 256)){
        uint8_t* pixelAddress = GetFrameBufferSegment() + 320*y + x;
        *pixelAddress = colorIndex;
    }

}

uint8_t Graphics::GetColorIndex(uint8_t r, uint8_t g, uint8_t b)
{
    if(r == 0x00, g == 0x00, b == 0xA8)
        return 0x01;
    return 0x00;
}

void Graphics::PutPixel(uint32_t x, uint32_t y,  uint8_t r, uint8_t g, uint8_t b)
{
    PutPixel(x,y, GetColorIndex(r,g,b));
}

void Graphics::ResetOffset()
{
    vga_x_offset = 0;
}

void Graphics::RenderBitMap(int bitmap[], uint8_t colorIndex, int x_offset, int y_offset)
{
    int x,y;
    int set;
    int mask;

    for (x=0; x < 8; x++) {
        for (y=0; y < 8; y++) {
            set = bitmap[x] & 1 << y;
            if (set != 0)
                PutPixel(x_offset+vga_x_offset+y, y_offset+x, colorIndex);
        }
    }
    vga_x_offset += 8;
}

void Graphics::Print(char* str, uint8_t colorIndex, int x_offset, int y_offset)
{
    if ((str[0] == '/') && (str[1] == '~')){
        vga_x_offset -= 8;
        RenderBitMap(font_basic[127], colorIndex);
        vga_x_offset -= 8;
        return;
    }

    int size = str_len(str);
    for (int i = 0; i < size; i++)
        RenderBitMap(font_basic[str[i]], colorIndex, x_offset, y_offset);
}