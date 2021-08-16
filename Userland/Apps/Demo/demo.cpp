#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/unistd.hpp>

int main()
{
    int result;
    char* buffer = (char*)malloc(sizeof(char) * 22);

    result = open((char*)"welcome");
    read(result, buffer, 22);
    close(result);
    printf("%s", buffer);

    free(buffer);
    exit(0);

    while (1)
        ;
}
