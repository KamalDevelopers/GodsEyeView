#include "language.hpp"

Language::Language()
{
    has_parsed_function = false;
    has_exit = false;
    script_index = 0;
    script_size = 0;

    program_arguments = (char**)malloc(sizeof(char*) * MAX_PROGRAM_ARGS);
    for (uint32_t i = 0; i < MAX_PROGRAM_ARGS; ++i) {
        program_arguments[i] = (char*)malloc(sizeof(char) * 50);
        memset(program_arguments[i], 0, 50);
    }
}

Language::~Language()
{
    for (uint32_t i = 0; i < MAX_PROGRAM_ARGS; i++)
        free(program_arguments[i]);
    free(program_arguments);
}

void Language::builtin_stat(char* file)
{
    if (!file) {
        printf("Missing file argument\n");
        return;
    }

    struct stat statbuffer;
    stat(file, &statbuffer);
    if (statbuffer.st_size == -1) {
        printf("Could not stat '%s'\n", file);
    } else {
        printf("uid: (%d) gid: (%d) \nname: %s\nsize: %d\n",
            statbuffer.st_uid, statbuffer.st_gid, file, statbuffer.st_size);
    }
}

void Language::builtin_exit()
{
    has_exit = true;
}

void Language::builtin_pwd()
{
    char cwd[100];
    getcwd(cwd);
    printf("/%s\n", cwd);
}

void Language::builtin_clear()
{
    clear();
}

void Language::builtin_reboot()
{
    _reboot();
}

void Language::builtin_shutdown()
{
    _shutdown();
}

void Language::builtin_uname()
{
    utsname uname_struct;
    uname(&uname_struct);
    printf("%s\n", uname_struct.sysname);
}

void Language::builtin_cd(char* dir)
{
    if (!dir) {
        printf("Missing directory argument\n");
        return;
    }

    if (chdir(dir) == -1) {
        printf("Path does not exist '%s'\n", dir);
        return;
    }

    cd_index++;
}

int Language::parse_token(char* token)
{
    if (!has_parsed_function && (strncmp(token, "exit", 4) == 0)) {
        builtin_exit();
        return 1;
    }

    if (!has_parsed_function && (strncmp(token, "stat", 4) == 0)) {
        char* file = strtok(NULL, " ");
        builtin_stat(file);
        return 1;
    }

    if (!has_parsed_function && (strncmp(token, "uname", 5) == 0)) {
        builtin_uname();
        return 1;
    }

    if (!has_parsed_function && (strncmp(token, "cd", 2) == 0)) {
        char* dir = strtok(NULL, " ");
        builtin_cd(dir);
        return 1;
    }

    if (!has_parsed_function && (strncmp(token, "clear", 5) == 0)) {
        builtin_clear();
        return 1;
    }

    if (!has_parsed_function && (strncmp(token, "pwd", 3) == 0)) {
        builtin_pwd();
        return 1;
    }

    if (!has_parsed_function && (strncmp(token, "shutdown", 8) == 0)) {
        builtin_shutdown();
        return 1;
    }

    if (!has_parsed_function && (strncmp(token, "reboot", 6) == 0)) {
        builtin_reboot();
        return 1;
    }

    if (!has_parsed_function) {
        strcpy(program, token);
        return 2;
    }

    strcpy(program_arguments[program_arguments_count], token);
    program_arguments_count++;
    return 0;
}

int Language::exec()
{
    char program_path[100];
    memset(program_path, 0, sizeof(program_path));

    int fd = open(program, O_RDONLY);
    if (fd == -1)
        strcat(program_path, "/bin/");
    close(fd);
    strcat(program_path, program);

    int pid = spawn(program_path, program_arguments, program_arguments_count);
    if (strcmp(program_arguments[0], "&") != 0)
        if (pid != -1)
            waitpid(pid);

    if (pid == -1)
        return -1;
    return 1;
}

int Language::run_line(const char* script)
{
    program_arguments_count = 0;
    has_parsed_function = 0;
    memset(program, 0, 100);
    for (uint32_t i = 0; i < MAX_PROGRAM_ARGS; ++i)
        memset(program_arguments[i], 0, 50);

    size_t script_index_start = script_index;
    while (script_index < script_size && script[script_index] != '\0' && script[script_index] != '\n')
        script_index++;
    if (script_index - script_index_start >= BUFSIZ)
        return -1;

    strncpy(script_line, script + script_index_start, script_index - script_index_start);
    char* token = strtok(script_line, " ");
    while (token && program_arguments_count < MAX_PROGRAM_ARGS) {
        int res = parse_token(token);
        if (res > 0)
            has_parsed_function = true;
        if (res == 1)
            return 1;

        token = strtok(NULL, " ");
    }

    return exec();
}

int Language::execute(const char* script, size_t size)
{
    script_index = 0;
    script_size = size;
    if (!size)
        return -1;

    int return_value = 0;
    while (script_index <= script_size && script[script_index] != 0) {
        int error = run_line(script);
        script_index++;

        if (error < 0) {
            printf("shell error: not found '%s'\n", program);
            return_value = -1;
        }
    }

    return return_value;
}
