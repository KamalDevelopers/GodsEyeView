#ifndef GDT_HPP
#define GDT_HPP

#include <LibC/types.hpp>

class GDT {
public:
    class SegDesc {
    private:
        uint16_t limit_lo;
        uint16_t base_lo;
        uint8_t base_hi;
        uint8_t type;
        uint8_t limit_hi;
        uint8_t base_vhi;

    public:
        SegDesc(uint32_t base, uint32_t limit, uint8_t type);
        uint32_t base();
        uint32_t limit();
    } __attribute__((packed));

private:
    SegDesc nullSegmentSelector;
    SegDesc unusedSegmentSelector;
    SegDesc codeSegmentSelector;
    SegDesc dataSegmentSelector;

public:
    GDT();
    ~GDT();

    uint16_t code_segment_selector();
    uint16_t data_segment_selector();
};

#endif
