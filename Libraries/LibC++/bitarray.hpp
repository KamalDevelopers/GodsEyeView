#ifndef BITARRAY_HPP
#define BITARRAY_HPP

#include <LibC/types.h>

template<uint32_t T>
class BitArray {
private:
    uint8_t bitmap[T / 8];

public:
    BitArray()
    {
        for (int i = 0; i < T / 8; i++)
            bitmap[i] = 0x00;
    }

    uint32_t size()
    {
        return T;
    }

    bool boundary(int index)
    {
        if (index >= size())
            return false;
        return true;
    }

    bool bit_set(int index)
    {
        if (!boundary(index))
            return false;
        bitmap[index / 8] |= 0x1 << (index % 8);
        return true;
    }

    bool bit_clear(int index)
    {
        if (!boundary(index))
            return false;
        bitmap[index / 8] &= ~(0x1 << (index % 8));
        return true;
    }

    bool bit_get(int index)
    {
        if (!boundary(index))
            return false;
        return (bitmap[index / 8] & (0x1 << (index % 0x8))) != 0x00;
    }

    bool bit_modify_range(uint32_t start, uint32_t size, bool set_or_clear)
    {
        if (!boundary(start) || !boundary(start + size))
            return false;

        for (uint32_t i = 0; i < size; i++) {
            if (set_or_clear)
                bit_set(start + i);
            else
                bit_clear(start + i);
        }

        return true;
    }

    bool bit_set_range(uint32_t start, uint32_t size)
    {
        return bit_modify_range(start, size, true);
    }

    bool bit_clear_range(uint32_t start, uint32_t size)
    {
        return bit_modify_range(start, size, false);
    }

    uint32_t find_unset(uint32_t length = 1)
    {
        uint32_t index = 0;
        for (; index < size(); index++) {
            bool fits = true;

            for (uint32_t i = 0; i < length; i++) {
                if (bit_get(index + i))
                    fits = false;
            }

            if (fits)
                break;
        }
        return index;
    }
};

#endif
