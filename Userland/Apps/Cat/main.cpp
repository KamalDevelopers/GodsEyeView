#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

bool print_file(char* file_name)
{
    int file_descriptor = open(file_name);
    if (file_descriptor < 0)
        return false;
    char buffer[1024];

    while (read(file_descriptor, buffer, sizeof(buffer)) != 0) {
        write(1, buffer, sizeof(buffer));
    }

    close(file_descriptor);
    return true;
}

int main(int argc, char** argv)
{
    if (!argc) {
        printf("No input file");
        return 0;
    }

    for (uint32_t i = 0; i < argc; i++) {
        if (print_file(argv[i]))
            continue;

        printf("File '%s' does not exist\n", argv[i]);
    }

    return 0;
}
