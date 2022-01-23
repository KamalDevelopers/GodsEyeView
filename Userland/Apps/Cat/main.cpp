#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

bool print_file(char* file_name)
{

    int file_descriptor;
    struct stat statbuffer;

    file_descriptor = open(file_name);
    fstat(file_descriptor, &statbuffer);

    if (statbuffer.st_size == -1) {
        return false;
    }

    char* buffer = (char*)malloc(sizeof(char) * statbuffer.st_size);

    read(file_descriptor, buffer, statbuffer.st_size);
    close(file_descriptor);

    printf("%s", buffer);
    free(buffer);
    return true;
}

int main(int argc, char** argv)
{
    if (!argc) {
        printf("No input file");
        return 0;
    }

    for (uint32_t i = 0; i < argc; i++) {
        if (!print_file(argv[i])) {
            printf("File '%s' does not exist", argv[i]);
            if (i != argc - 1)
                printf("\n");
        }
    }

    return 0;
}
