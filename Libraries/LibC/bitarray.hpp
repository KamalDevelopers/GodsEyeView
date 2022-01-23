#include "types.hpp"

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

    bool boundary(uint32_t index)
    {
        if (index >= T)
            return false;
        return true;
    }

    bool bit_set(uint32_t index)
    {
        if (!boundary(index))
            return false;
        bitmap[index / 8] |= 0x1 << (index % 8);
        return true;
    }

    bool bit_clear(uint32_t index)
    {
        if (!boundary(index))
            return false;

        bitmap[index / 8] &= ~(0x1 << (index % 8));
        return true;
    }

    bool bit_get(uint32_t index)
    {
        if (!boundary(index))
            return false;
        return (bitmap[index / 8] & (0x1 << (index % 0x8))) != 0x00;
    }

    bool bit_do_range(uint32_t start, uint32_t size, bool set_or_clear)
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
        return bit_do_range(start, size, true);
    }

    bool bit_clear_range(uint32_t start, uint32_t size)
    {
        return bit_do_range(start, size, false);
    }

    uint32_t find_unset(uint32_t size = 1)
    {
        uint32_t index = 0;
        for (; index < T; index++) {
            bool fits = true;

            for (uint32_t s = 0; s < size; s++) {
                if (bit_get(index + s))
                    fits = false;
            }

            if (fits)
                break;
        }
        return index;
    }
};
