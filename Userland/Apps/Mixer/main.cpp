#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

int number_places(int n)
{
    if (n < 0)
        return number_places((n == INT_MIN) ? INT_MAX : -n);
    if (n < 10)
        return 1;
    return 1 + number_places(n / 10);
}

bool play(char* file_name)
{
    int audio_device = open((char*)"/dev/audio");
    int file_descriptor = open(file_name);
    struct stat statbuffer;

    fstat(file_descriptor, &statbuffer);
    if (statbuffer.st_size == -1) {
        return false;
    }

    uint8_t* data = (uint8_t*)malloc(statbuffer.st_size);
    read(file_descriptor, data, statbuffer.st_size);
    close(file_descriptor);
    uint32_t size = statbuffer.st_size - 4096;

    printf("Playing %s at 8000 Hz ", file_name);
    uint32_t number_places_size = number_places(size / 4096);
    uint8_t* sample_data = (uint8_t*)malloc(4096);

    for (uint32_t i = 0; i < size; i += 4096) {
        memcpy(sample_data, data + i, 4096);
        write(audio_device, sample_data, 4096);
        printf("[%d:%d]", i / 4096, size / 4096);
        flush();

        uint32_t position = (i == 0) ? 1 : i;
        for (uint32_t x = 0; x < number_places_size + number_places(position / 4096) + 3; x++)
            printf("%s", "\b");
    }

    printf("[done]");

    free(data);
    free(sample_data);

    return true;
}

int main(int argc, char** argv)
{
    if (!argc) {
        printf("No input file");
        return 0;
    }

    for (uint32_t i = 0; i < argc; i++) {
        if (!play(argv[i])) {
            printf("File '%s' does not exist", argv[i]);
        }
        if (i != argc - 1)
            printf("\n");
    }

    return 0;
}
