#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibC/unistd.hpp>
#include <LibC/utsname.hpp>

int command(char* input)
{
    char** arguments = (char**)malloc(sizeof(char*) * 10);
    for (uint32_t i = 0; i < 10; ++i)
        arguments[i] = (char*)malloc(50);

    for (uint32_t i = 0; i < 10; i++)
        memset(arguments[i], 0, 50);

    char* program = strtok(input, (char*)" ");

    if (strcmp(program, "uname") == 0) {
        utsname uname_struct;
        uname(&uname_struct);
        printf("%s", uname_struct.sysname);
        return 1;
    }

    if (strcmp(program, "stat") == 0) {
        char* file = strtok(NULL, (char*)" ");
        if (!file) {
            printf("Missing file argument");
            return 1;
        }
        struct stat statbuffer;

        stat(file, &statbuffer);
        if (statbuffer.st_size == -1) {
            printf("Could not stat '%s'", file);
        } else {
            printf("Uid: (%d) Gid: (%d) \nName: %s\nSize: %d",
                statbuffer.st_uid, statbuffer.st_gid, file, statbuffer.st_size);
        }
        return 1;
    }

    char* arg = strtok(NULL, (char*)" ");

    uint32_t argc = 0;
    while (arg) {
        if (arg) {
            strcpy(arguments[argc], arg);
            argc++;
        }
        arg = strtok(NULL, (char*)" ");
    }

    if (strcmp(program, "clear") == 0) {
        clear();
        return 2;
    }

    if (strcmp(program, "echo") == 0) {
        for (uint32_t i = 0; i < 10; i++)
            printf("%s ", arguments[i]);
        flush();
        return 1;
    }

    if (strcmp(program, "shutdown") == 0) {
        _shutdown();
    }

    if (strcmp(program, "reboot") == 0) {
        _reboot();
    }

    int pid = spawn(program, arguments);
    if (pid != -1) {
        waitpid(pid);
        return 1;
    }

    for (uint32_t i = 0; i < 10; i++)
        free(arguments[i]);
    free(arguments);
    return 0;
}

int main(int argc, char** argv)
{
    const char ps1[] = "\33\x2\x9%s\33\x3@\33\x2\x0C%s\33\x3:\33\x2\xA%s\33\x3# ";

    utsname uname_struct;
    uname(&uname_struct);

    const char* user = "terry";
    const char* path = "/";
    lowercase(uname_struct.sysname);

    while (1) {
        printf(ps1, user, uname_struct.sysname, path);
        flush();

        char input[100];
        int read_size = read(0, input, 100);
        input[read_size - 1] = 0;

        if (!strlen(input))
            continue;

        int result = command(input);
        if (result == 0)
            printf("Unknown command '%s'", input);
        if (result != 2)
            printf("\n");
    }

    return 0;
}
