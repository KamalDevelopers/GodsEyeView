#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/unistd.hpp>

int main()
{
    int file_descriptor;
    struct stat statbuffer;

    file_descriptor = open((char*)"welcome");
    fstat(file_descriptor, &statbuffer);
    char* buffer = (char*)malloc(sizeof(char) * statbuffer.st_size);

    read(file_descriptor, buffer, statbuffer.st_size);
    close(file_descriptor);
    printf("%s", buffer);

    free(buffer);
    return 0;
}
