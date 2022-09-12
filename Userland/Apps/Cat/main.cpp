#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>

bool print_file(char* file_name)
{
    int file_descriptor = open(file_name, O_RDONLY);
    if (file_descriptor < 0)
        return false;

    char buffer[BUFSIZ];
    int size = 1;
    memset(buffer, 0, sizeof(buffer));

    while (size > 0) {
        size = read(file_descriptor, buffer, sizeof(buffer));
        write(1, buffer, size);
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

        if (i != 0)
            printf("\n");
        printf("File '%s' does not exist", argv[i]);
    }

    return 0;
}
