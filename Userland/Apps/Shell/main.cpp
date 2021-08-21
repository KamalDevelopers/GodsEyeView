#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibC/unistd.hpp>
#include <LibC/utsname.hpp>

int command(char* input)
{
    char* program = strtok(input, (char*)" ");
    if (strcmp(program, "uname") == 0) {
        utsname uname_struct;
        uname(&uname_struct);
        printf("%s\n", uname_struct.sysname);
        return 1;
    }

    if (strcmp(program, "stat") == 0) {
        char* file = strtok(NULL, (char*)" ");
        if (!file) {
            printf("Missing file argument\n");
            return 1;
        }
        struct stat statbuffer;

        stat(file, &statbuffer);
        if (statbuffer.st_size == -1) {
            printf("Could not stat '%s'\n", file);
        } else {
            printf("Uid: (%d) Gid: (%d) \nName: %s\nSize: %d\n",
                statbuffer.st_uid, statbuffer.st_gid, file, statbuffer.st_size);
        }
        return 1;
    }

    if (strcmp(program, "clear") == 0) {
        clear();
        return 1;
    }

    if (strcmp(program, "shutdown") == 0) {
        _shutdown();
    }

    if (strcmp(program, "reboot") == 0) {
        _reboot();
    }

    return 0;
}

int main()
{
    const char ps1[] = "\33\x2\x9%s\33\x3@\33\x2\x0C%s\33\x3:\33\x2\xA%s\33\x3# ";

    utsname uname_struct;
    uname(&uname_struct);

    const char* user = "terry";
    const char* path = "/";
    lowercase(uname_struct.sysname);

    while (1) {
        printf(ps1, uname_struct.sysname, user, path);

        char input[100];
        read(0, input, 99);

        if (!strlen(input))
            continue;

        if (command(input) == 0)
            printf("Unknown command '%s'\n", input);
    }

    exit(0);
    while (1)
        ;
}
