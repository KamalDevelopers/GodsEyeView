#include "wave.hpp"

Wave::Wave(char* file)
{
    int file_descriptor = open(file, O_RDONLY);
    struct stat statbuffer;

    fstat(file_descriptor, &statbuffer);
    if (statbuffer.st_size == -1)
        return;

    uint32_t size = statbuffer.st_size;
    uint8_t* data = (uint8_t*)malloc(size);
    uint8_t* orig_data = data;
    read(file_descriptor, data, size);
    close(file_descriptor);

    memcpy(&riff_header, data, sizeof(riff_header_t));
    data += sizeof(riff_header_t);
    memcpy(&fmt_header, data, sizeof(fmt_header_t));
    data += fmt_header.chunk_size + 8;

    if (strncmp((const char*)data, "LIST", 4) == 0) {
        memcpy(&list_header, data, sizeof(list_header_t));
        data += list_header.chunk_size + 8;
    }

    memcpy(&data_header, data, sizeof(data_header_t));
    data += sizeof(data_header_t);

    if (!validate())
        return;

    sample_data = (uint8_t*)malloc(data_header.size);
    memcpy(sample_data, data, data_header.size);
    free(orig_data);
}

Wave::~Wave()
{
    if (sample_data && free_samples)
        free(sample_data);
}

bool Wave::validate()
{
    if (strncmp(riff_header.magic, "RIFF", 4) != 0)
        valid = false;
    if (strncmp(riff_header.magic_wave, "WAVE", 4) != 0)
        valid = false;
    if (strncmp(fmt_header.magic, "fmt ", 4) != 0)
        valid = false;
    return valid;
}

void Wave::disown_data()
{
    free_samples = false;
}
