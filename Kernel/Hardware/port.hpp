#ifndef PORT_HPP
#define PORT_HPP

#include <LibC/types.hpp>

uint32_t inbl(uint16_t _port);
void outbl(uint16_t _port, uint32_t _data);
void outbw(uint16_t port, uint16_t data);
uint16_t inbw(uint16_t port);
void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
void delay(uint32_t microseconds);

class Port {
protected:
    Port(uint16_t portnumber);
    ~Port();
    uint16_t portnumber;
};

class Port8Bit : public Port {
public:
    Port8Bit(uint16_t portnumber);
    ~Port8Bit();

    virtual uint8_t read();
    virtual void write(uint8_t data);

protected:
    static inline uint8_t read8(uint16_t port)
    {
        uint8_t result;
        __asm__ volatile("inb %1, %0"
                         : "=a"(result)
                         : "Nd"(port));
        return result;
    }

    static inline void write8(uint16_t port, uint8_t data)
    {
        __asm__ volatile("outb %0, %1"
                         :
                         : "a"(data), "Nd"(port));
    }
};

class Port8BitSlow : public Port8Bit {
public:
    Port8BitSlow(uint16_t portnumber);
    ~Port8BitSlow();

    virtual void write(uint8_t data);

protected:
    static inline void write8slow(uint16_t port, uint8_t data)
    {
        __asm__ volatile("outb %0, %1\njmp 1f\n1: jmp 1f\n1:"
                         :
                         : "a"(data), "Nd"(port));
    }
};

class Port16Bit : public Port {
public:
    Port16Bit(uint16_t portnumber);
    ~Port16Bit();

    virtual uint16_t read();
    virtual void write(uint16_t data);

protected:
    static inline uint16_t read16(uint16_t port)
    {
        uint16_t result;
        __asm__ volatile("inw %1, %0"
                         : "=a"(result)
                         : "Nd"(port));
        return result;
    }

    static inline void write16(uint16_t port, uint16_t data)
    {
        __asm__ volatile("outw %0, %1"
                         :
                         : "a"(data), "Nd"(port));
    }
};

class Port32Bit : public Port {
public:
    Port32Bit(uint16_t portnumber);
    ~Port32Bit();

    virtual uint32_t read();
    virtual void write(uint32_t data);

protected:
    static inline uint32_t read32(uint16_t port)
    {
        uint32_t result;
        __asm__ volatile("inl %1, %0"
                         : "=a"(result)
                         : "Nd"(port));
        return result;
    }

    static inline void write32(uint16_t port, uint32_t data)
    {
        __asm__ volatile("outl %0, %1"
                         :
                         : "a"(data), "Nd"(port));
    }
};

#endif
