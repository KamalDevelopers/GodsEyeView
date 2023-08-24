#ifndef SHELL_HPP
#define SHELL_HPP

#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>
#include <LibC/utsname.h>

#define PS1 "\33\x2\xC%s\33\x3@\33\x2\xF%s\33\x3:\33\x2\xC%s\33\x3# "
#define KEY_TAB 9
#define KEY_ENTER 10
#define MAX_AUTOCOMP_WORDS 100

class Shell {
private:
    utsname uname_struct;
    uint32_t input_line_index = 0;
    uint32_t autocomplete_word_size = 0;
    uint32_t autocomplete_table_size = 0;
    int autocomplete_word = -1;
    int autocomplete_input_skip = 0;
    char input_line_buffer[100];
    char cwd[100];
    char user[25];
    char* autocomplete_table[MAX_AUTOCOMP_WORDS];

    void flush_chars(int size);
    void flush_autocomplete(int start);
    uint8_t handle_input_line_key();

public:
    Shell();
    ~Shell();

    const char* input_line() { return input_line_buffer; }
    void autocomplete_table_builder();
    int match_autocomplete(const char* word, size_t word_size);
    uint8_t append_autocomplete_word(const char* word);
    void exec(const char* script);
    size_t read_input_line();
    void write_prompt();
};

#endif
