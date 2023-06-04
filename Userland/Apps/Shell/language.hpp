#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>
#include <LibC/utsname.h>

#define MAX_PROGRAM_ARGS 10

class Language {
private:
    size_t script_index = 0;
    size_t script_size = 0;
    char script_line[BUFSIZ];
    char program[100];
    char** program_arguments = 0;
    uint32_t program_arguments_count = 0;
    bool has_parsed_function = false;
    bool has_exit = false;

    int parse_token(char* token);
    void builtin_cd(char* dir);
    void builtin_stat(char* file);
    void builtin_pwd();
    void builtin_uname();
    void builtin_exit();
    void builtin_shutdown();
    void builtin_reboot();
    void builtin_clear();
    int exec();

public:
    Language();
    ~Language();

    bool should_exit() { return has_exit; }
    int run_line(const char* script);
    int execute(const char* script, size_t size);
};

#endif
