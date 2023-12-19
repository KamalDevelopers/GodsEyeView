#include <LibC/ctype.h>
#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>

int err(const char* arg)
{
    char num[4];
    size_t len = strlen(arg);
    if (len >= sizeof(num))
        return -1;
    strncpy(num, arg, len);
    for (uint8_t i = 0; i < len; ++i)
        if (!isdigit(num[i]))
            return -1;
    int e = atoi(num);
    printf("(%d) %s\n", e, error_what(e));
    return 0;
}

int main(int argc, char** argv)
{
    if (argc && (strncmp(argv[0], "err", 3) == 0)) {
        if (err(argv[1]) < 0)
            printf("Error not defined\n");
        return 0;
    }

    int fd = open("/dev/klog", O_RDONLY);

    char buffer[1024];
    printf("\34\x2\xF");
    flush();
    memset(buffer, 0, sizeof(buffer));
    if (read(fd, buffer, sizeof(buffer))) {
        printf("%s\n", buffer);
    }

    flush();
    printf("\34\x3");
    close(fd);
    return 0;
}
