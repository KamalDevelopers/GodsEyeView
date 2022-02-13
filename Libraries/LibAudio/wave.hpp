#ifndef WAVE_HPP
#define WAVE_HPP

#include "dr_wav/dr_wav.h"
#include <LibC/stat.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibC/unistd.hpp>

class Wave {
private:
    uint32_t sample_rate = 0;
    uint32_t sample_count = 0;
    uint32_t channels = 0;

    uint8_t* buffer = 0;
    size_t buffer_size = 0;
    drwav wav;

    bool init();

public:
    Wave(char* file = 0);
    ~Wave();

    uint32_t get_sample_rate() { return sample_rate; }
    uint32_t get_sample_count() { return sample_count; }
    uint32_t get_channels() { return channels; }

    bool load(uint8_t* data, size_t size);
    void read_pcm_16(uint8_t* buffer);
};

#endif
