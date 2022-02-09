#ifndef GDT_HPP
#define GDT_HPP

#include <LibC/types.hpp>

class SegmentDescriptor {
private:
    uint16_t limit_lo;
    uint16_t base_lo;
    uint8_t base_hi;
    uint8_t type;
    uint8_t limit_hi;
    uint8_t base_vhi;

public:
    SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type);
    uint32_t base();
    uint32_t limit();
} __attribute__((packed));

class GDT {
private:
    SegmentDescriptor null_segment_selector;
    SegmentDescriptor unused_segment_selector;
    SegmentDescriptor code_segment_selector;
    SegmentDescriptor data_segment_selector;

public:
    GDT();
    ~GDT();

    static GDT* active;

    uint16_t get_code_segment_selector();
    uint16_t get_data_segment_selector();
};

#endif
