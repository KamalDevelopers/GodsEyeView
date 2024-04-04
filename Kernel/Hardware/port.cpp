#include "port.hpp"

void delay(uint32_t microseconds)
{
    for (uint32_t i = 0; i < microseconds; ++i)
        inb(0x80);
}

uint32_t inbl(uint16_t _port)
{
    uint32_t result;
    asm volatile("inl %%dx, %%eax"
                 : "=a"(result)
                 : "dN"(_port));
    return result;
}

void outbl(uint16_t _port, uint32_t _data)
{
    asm volatile("outl %%eax, %%dx"
                 :
                 : "dN"(_port), "a"(_data));
}

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

void outb(uint16_t port, uint8_t data)
{
    asm volatile("outb %0, %1"
                 :
                 : "a"(data), "Nd"(port));
}

uint8_t inb(uint16_t port)
{
    uint8_t result;
    asm volatile("inb %1, %0"
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

void Port8Bit::write(uint8_t data)
{
    write8(portnumber, data);
}

void Port8Bit::write_fast(uint8_t data)
{
    outb(portnumber, data);
}

uint8_t Port8Bit::read()
{
    return read8(portnumber);
}

Port8BitSlow::Port8BitSlow(uint16_t portnumber)
    : Port8Bit(portnumber)
{
}

Port8BitSlow::~Port8BitSlow()
{
}

void Port8BitSlow::write(uint8_t data)
{
    write8slow(portnumber, data);
}

Port16Bit::Port16Bit(uint16_t portnumber)
    : Port(portnumber)
{
}

Port16Bit::~Port16Bit()
{
}

void Port16Bit::write(uint16_t data)
{
    write16(portnumber, data);
}

uint16_t Port16Bit::read()
{
    return read16(portnumber);
}

Port32Bit::Port32Bit(uint16_t portnumber)
    : Port(portnumber)
{
}

Port32Bit::~Port32Bit()
{
}

void Port32Bit::write(uint32_t data)
{
    write32(portnumber, data);
}

uint32_t Port32Bit::read()
{
    return read32(portnumber);
}
