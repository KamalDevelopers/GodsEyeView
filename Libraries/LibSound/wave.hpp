#ifndef WAVE_HPP
#define WAVE_HPP

#include <LibC/liballoc.hpp>
#include <LibC/stat.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibC/unistd.hpp>

typedef struct riff_header {
    char magic[4];
    uint32_t chunk_size = 0;
    char magic_wave[4];
} __attribute__((packed)) riff_header_t;

typedef struct fmt_header {
    char magic[4];
    uint32_t chunk_size = 0;
    uint16_t audio_format = 0;
    uint16_t channels = 0;
    uint32_t sample_rate = 0;
    uint32_t byte_rate = 0;
    uint16_t block_align = 0;
    uint16_t bits_per_sample = 0;
} __attribute__((packed)) fmt_header_t;

typedef struct list_header {
    char magic[4];
    uint32_t chunk_size = 0;
    uint32_t type = 0;
} __attribute__((packed)) list_header_t;

typedef struct data_header {
    char magic[4];
    uint32_t size = 0;
} __attribute__((packed)) data_header_t;

class Wave {
private:
    riff_header_t riff_header;
    fmt_header_t fmt_header;
    list_header_t list_header;
    data_header_t data_header;
    uint8_t* sample_data = 0;
    bool free_samples = true;
    bool valid = true;

public:
    Wave(char* file);
    ~Wave();

    uint32_t size() { return data_header.size; }
    uint8_t* samples() { return sample_data; }
    uint8_t* take_samples();
    bool is_valid() { return valid; }
    bool validate();
};

#endif
