#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>

bool print_file(char* file_name)
{
    int file_descriptor = open(file_name, O_RDONLY);
    if (file_descriptor < 0)
        return false;

    static char buffer[BUFSIZ];
    int size = 1;
    bool is_first = true;

    while (size > 0) {
        size = read(file_descriptor, buffer, sizeof(buffer));
        write(1, buffer, size);

        if (size && !is_first) {
            flush();
            usleep(100);
        }

        is_first = false;
    }

    close(file_descriptor);
    return true;
}

int main(int argc, char** argv)
{
    if (!argc) {
        printf("Usage: cat [file(s)]\n");
        return 0;
    }

    for (uint32_t i = 0; i < argc; i++) {
        if (print_file(argv[i]))
            continue;

        if (i != 0)
            printf("\n");
        printf("File '%s' does not exist\n", argv[i]);
    }

    return 0;
}
