#include "port.hpp"

void outbw(uint16_t port, uint16_t data)
{
    asm volatile("outw %0, %1"
                 :
                 : "a"(data), "Nd"(port));
}

uint16_t inbw(uint16_t port)
{
    uint16_t result;
    asm volatile("inw %1, %0"
                 : "=a"(result)
                 : "Nd"(port));
    return result;
}

Port::Port(uint16_t portnumber)
{
    this->portnumber = portnumber;
}

Port::~Port()
{
}

Port8Bit::Port8Bit(uint16_t portnumber)
    : Port(portnumber)
{
}

Port8Bit::~Port8Bit()
{
}

void Port8Bit::Write(uint8_t data)
{
    Write8(portnumber, data);
}

uint8_t Port8Bit::Read()
{
    return Read8(portnumber);
}

Port8BitSlow::Port8BitSlow(uint16_t portnumber)
    : Port8Bit(portnumber)
{
}

Port8BitSlow::~Port8BitSlow()
{
}

void Port8BitSlow::Write(uint8_t data)
{
    Write8Slow(portnumber, data);
}

Port16Bit::Port16Bit(uint16_t portnumber)
    : Port(portnumber)
{
}

Port16Bit::~Port16Bit()
{
}

void Port16Bit::Write(uint16_t data)
{
    Write16(portnumber, data);
}

uint16_t Port16Bit::Read()
{
    return Read16(portnumber);
}

Port32Bit::Port32Bit(uint16_t portnumber)
    : Port(portnumber)
{
}

Port32Bit::~Port32Bit()
{
}

void Port32Bit::Write(uint32_t data)
{
    Write32(portnumber, data);
}

uint32_t Port32Bit::Read()
{
    return Read32(portnumber);
}
