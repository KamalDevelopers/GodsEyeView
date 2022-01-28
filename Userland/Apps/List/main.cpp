#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

bool is_dir(char* name)
{
    int fd = open(name);
    close(fd);

    if (fd == -1)
        return true;
    return false;
}

int read_dir(char* name, bool root)
{
    char** names = (char**)malloc(sizeof(char*) * 50);
    for (uint32_t i = 0; i < 50; ++i)
        names[i] = (char*)malloc(100);

    int exists = -1;
    if (!root) {
        if (name[strlen(name) - 1] != '/')
            strcat(name, "/");
    }
    exists = listdir(name, (char**)names);

    if (exists == -1) {
        printf("Folder does not exist");
        return -1;
    }

    for (uint32_t i = 0; i < 10; i++) {
        if (!strlen(names[i]))
            continue;

        if (is_dir(names[i])) {
            printf("\33\x2\x9%s\33\x3 ", names[i]);
            continue;
        }

        printf("%s ", names[i]);
    }

    for (uint32_t i = 0; i < 50; i++)
        free(names[i]);
    free(names);
    return 0;
}

int main(int argc, char** argv)
{
    if (!argc)
        read_dir("", true);
    else
        read_dir(argv[0], false);

    return 0;
}
