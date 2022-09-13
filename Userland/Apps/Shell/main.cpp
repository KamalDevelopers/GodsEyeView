#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>
#include <LibC/unistd.h>
#include <LibC/utsname.h>

static char current_path[100];

int command(char* input)
{
    char* program = strtok(input, " ");

    if (strcmp(program, "uname") == 0) {
        utsname uname_struct;
        uname(&uname_struct);
        printf("%s", uname_struct.sysname);
        return 1;
    }

    if (strcmp(program, "cd") == 0) {
        char* dir = strtok(NULL, " ");
        if (!dir) {
            printf("Missing directory argument");
            return 1;
        }

        if (chdir(dir) == -1) {
            printf("Path does not exist '%s'", dir);
            return 1;
        }
        return 2;
    }

    if (strcmp(program, "stat") == 0) {
        char* file = strtok(NULL, " ");
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

    if (strcmp(program, "clear") == 0) {
        clear();
        return 2;
    }

    if (strcmp(program, "shutdown") == 0)
        _shutdown();

    if (strcmp(program, "reboot") == 0)
        _reboot();

    char* arg = strtok(NULL, " ");

    char** arguments = (char**)malloc(sizeof(char*) * 10);
    for (uint32_t i = 0; i < 10; ++i) {
        arguments[i] = (char*)malloc(50);
        memset(arguments[i], 0, 50);
    }

    uint32_t argc = 0;
    while (arg) {
        if (arg) {
            strcpy(arguments[argc], arg);
            argc++;
        }
        arg = strtok(NULL, " ");
    }

    char program_path[100];
    memset(program_path, 0, sizeof(program_path));

    int fd = open(program, O_RDONLY);
    if (fd == -1)
        strcat(program_path, "/bin/");
    close(fd);
    strcat(program_path, program);

    int pid = spawn(program_path, arguments);
    if (strcmp(arguments[0], "&") != 0)
        if (pid != -1)
            waitpid(pid);

    for (uint32_t i = 0; i < 10; i++)
        free(arguments[i]);
    free(arguments);

    if (pid == -1)
        return 0;
    return 1;
}

int main(int argc, char** argv)
{
    const char ps1[] = "\33\x2\xC%s\33\x3@\33\x2\xF%s\33\x3:\33\x2\xC/%s\33\x3# ";

    utsname uname_struct;
    uname(&uname_struct);

    const char* user = "terry";
    lowercase(uname_struct.sysname);
    char input[100];

    while (1) {
        getcwd(current_path);
        printf(ps1, user, uname_struct.sysname, current_path);
        flush();

        memset(input, 0, sizeof(input));
        int read_size = read(0, input, sizeof(input));
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
