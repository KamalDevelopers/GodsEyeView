#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

int main(int argc, char** argv)
{
    if (!argc) {
        printf("No input file");
        return 0;
    }

    int file_descriptor;
    struct stat statbuffer;

    file_descriptor = open((char*)argv[0]);
    fstat(file_descriptor, &statbuffer);

    if (statbuffer.st_size == -1) {
        printf("File '%s' does not exist", (char*)argv[0]);
        return 0;
    }

    char* buffer = (char*)malloc(sizeof(char) * statbuffer.st_size);

    read(file_descriptor, buffer, statbuffer.st_size);
    close(file_descriptor);

    printf("%s", buffer);
    free(buffer);
    return 0;
}
