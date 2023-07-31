#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>

int main(int argc, char** argv)
{
    int fd = open("/dev/klog", O_RDONLY);

    char buffer[1024];
    printf("\33\x2\xF");
    flush();
    memset(buffer, 0, sizeof(buffer));
    if (read(fd, buffer, sizeof(buffer))) {
        printf("%s\n", buffer);
    }

    flush();
    printf("\33\x3");
    close(fd);

    return 0;
}
