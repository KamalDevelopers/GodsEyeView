#ifndef VGA_HPP
#define VGA_HPP
#include "LibC/stdlib.hpp"
#include "LibC/stdio.hpp"
#include "LibC/types.hpp"
#include "../port.hpp"

#define VGA_BLUE           0x1
#define VGA_GREEN          0x2
#define VGA_CYAN           0x3
#define VGA_RED            0x4
#define VGA_MAGENTA        0x5
#define VGA_BROWN          0x6
#define VGA_GRAY           0x7

class Graphics
{
protected:
	Port8Bit miscPort;
	Port8Bit crtcIndexPort;
	Port8Bit crtcDataPort;
	Port8Bit sequencerIndexPort;
	Port8Bit sequencerDataPort;
	Port8Bit graphicsControllerIndexPort;
	Port8Bit graphicsControllerDataPort;
	Port8Bit attributeControllerIndexPort;
	Port8Bit attributeControllerReadPort;
	Port8Bit attributeControllerWritePort;
	Port8Bit attributeControllerResetPort;

	void WriteRegisters(uint8_t* registers);
	uint8_t* GetFrameBufferSegment();

	virtual uint8_t GetColorIndex(uint8_t r, uint8_t g, uint8_t b);

public:
	Graphics();
	~Graphics();

	virtual bool Init(uint32_t width, uint32_t height, uint32_t colordepth, uint8_t colorIndex);
	virtual bool SetMode(uint32_t width, uint32_t height, uint32_t colordepth);
	virtual void PutPixel(uint32_t x, uint32_t y,  uint8_t r, uint8_t g, uint8_t b);
	virtual void PutPixel(uint32_t x, uint32_t y, uint8_t colorIndex);
	virtual void RenderBitMap(int bitmap[], uint8_t colorIndex, unsigned int gridX, unsigned int gridY);
private:
	int vga_x_offset = 0;
	int vga_y_offset = 0;

};
#endif