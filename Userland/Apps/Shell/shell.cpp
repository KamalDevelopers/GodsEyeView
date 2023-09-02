#include "shell.hpp"

Shell::Shell()
{
    input_line_index = 0;
    autocomplete_word_size = 0;
    autocomplete_word = -1;
    autocomplete_table_size = 0;

    memset(user, 0, sizeof(user));
    memset(cwd, 0, sizeof(cwd));
    strncpy(user, "terry", 5);
    uname(&uname_struct);
    lowercase(uname_struct.sysname);
    autocomplete_table_builder();
}

Shell::~Shell()
{
    for (uint32_t i = 0; i < autocomplete_table_size; i++)
        free(autocomplete_table[i]);
}

void Shell::autocomplete_table_builder()
{
    for (uint32_t i = 0; i < autocomplete_table_size; i++)
        free(autocomplete_table[i]);
    autocomplete_table_size = 0;

    getcwd(cwd);
    fs_entry_t* entries = (fs_entry_t*)malloc(sizeof(fs_entry_t) * 100);

    append_autocomplete_word("ls");
    append_autocomplete_word("pwd");

    int count = listdir("/bin/", entries, 100);
    if (count > 0) {
        for (uint32_t i = 0; i < count; i++)
            append_autocomplete_word(entries[i].name);
    }

    append_autocomplete_word("shutdown");
    append_autocomplete_word("reboot");
    append_autocomplete_word("clear");
    autocomplete_skip_bins = count + 5;

    count = listdir(cwd, entries, 100);
    if (count > 0) {
        for (uint32_t i = 0; i < count; i++)
            append_autocomplete_word(entries[i].name);
    }

    free(entries);
}

int Shell::match_autocomplete(const char* word, size_t word_size)
{
    for (uint32_t t = 0; t < autocomplete_table_size; t++) {
        bool match = true;
        if (t < autocomplete_skip_bins && !is_first_word)
            match = false;
        for (uint32_t i = 0; i < word_size; i++) {
            if ((autocomplete_table[t][i] == 0) || (word[i] != autocomplete_table[t][i])) {
                match = false;
                break;
            }
        }
        if (match)
            return t;
    }

    return -1;
}

uint8_t Shell::append_autocomplete_word(const char* word)
{
    if (autocomplete_table_size >= MAX_AUTOCOMP_WORDS)
        return 0;

    int size = strlen(word);
    autocomplete_table[autocomplete_table_size] = (char*)malloc(size * sizeof(char));
    strncpy(autocomplete_table[autocomplete_table_size], word, size);
    autocomplete_table_size++;
    return 1;
}

void Shell::write_prompt()
{
    getcwd(cwd);
    printf(PS1, user, uname_struct.sysname, cwd);
    flush();
}

void Shell::flush_chars(int size)
{
    if (!size)
        return;
    for (uint8_t i = 0; i < size; i++)
        printf("\b");
}

void Shell::flush_autocomplete(int start)
{
    if (!autocomplete_word_size && start > 0)
        return;

    for (int z = start; z < autocomplete_word_size; z++)
        printf(" ");
    for (int z = start; z < autocomplete_word_size; z++)
        printf("\b");
}

uint8_t Shell::handle_input_line_key()
{
    if (input_line_buffer[input_line_index - 1] == ' ') {
        is_first_word = 0;
        flush_autocomplete(input_line_index - 1 - autocomplete_input_skip);
        autocomplete_word_size = 0;
        autocomplete_input_skip = input_line_index;
    }

    if (input_line_buffer[input_line_index - 1] == KEY_ENTER) {
        flush_autocomplete(0);
        printf("\n");
        return 2;
    }

    if (input_line_buffer[input_line_index - 1] == KEY_TAB) {
        input_line_index--;
        input_line_buffer[input_line_index] = 0;
        if (autocomplete_word == -1 || !autocomplete_word_size)
            return 1;
        if (input_line_index >= 1 && input_line_buffer[input_line_index - 1] == ' ')
            return 1;

        is_first_word = 0;
        flush_chars(input_line_index - autocomplete_input_skip);
        flush_autocomplete(0);
        memset(input_line_buffer + autocomplete_input_skip, 0, sizeof(input_line_buffer) - autocomplete_input_skip);
        input_line_index = strlen(autocomplete_table[autocomplete_word]);
        strcat(input_line_buffer + autocomplete_input_skip, autocomplete_table[autocomplete_word]);
        input_line_buffer[input_line_index + autocomplete_input_skip] = 0;
        if (!autocomplete_input_skip) {
            input_line_buffer[input_line_index + autocomplete_input_skip] = ' ';
            input_line_buffer[input_line_index + 1 + autocomplete_input_skip] = 0;
            input_line_index++;
        }
        printf("%s", input_line_buffer + autocomplete_input_skip);
        autocomplete_word = -1;
        autocomplete_word_size = 0;
        autocomplete_word_size = 0;
        input_line_index += autocomplete_input_skip;
        autocomplete_input_skip = input_line_index;
        return 1;
    }

    if (input_line_buffer[input_line_index - 1] == '\b') {
        if (input_line_index > 1) {
            input_line_index -= 2;
            input_line_buffer[input_line_index] = 0;
            input_line_buffer[input_line_index + 1] = 0;
            printf("\b");
            flush_autocomplete(input_line_index + autocomplete_input_skip);
            if (input_line_index <= autocomplete_input_skip) {
                flush_autocomplete(0);
                autocomplete_input_skip = 0;
                is_first_word = 1;
            }
            if (input_line_index > 2 && input_line_buffer[input_line_index - 2] == ' ') {
                flush_autocomplete(0);
                is_first_word = 1;
            }
            autocomplete_word_size = 0;
            autocomplete_word = -1;
        } else {
            is_first_word = 1;
            input_line_buffer[input_line_index - 1] = 0;
            input_line_index--;
        }
    } else {
        printf("%c", input_line_buffer[input_line_index - 1]);
    }

    autocomplete_word = match_autocomplete(input_line_buffer + autocomplete_input_skip, input_line_index - autocomplete_input_skip);

    /* no autocomplete word */
    if (autocomplete_word == -1 || ((input_line_index - autocomplete_input_skip) <= 0)) {
        flush_autocomplete(input_line_index - autocomplete_input_skip);
        autocomplete_word_size = 0;
        return 1;
    }

    /* print autocomplete word */
    int new_word_size = strlen(autocomplete_table[autocomplete_word]);
    if (new_word_size < autocomplete_word_size)
        flush_autocomplete(input_line_index - autocomplete_input_skip);
    if (new_word_size == autocomplete_word_size)
        return 0;
    autocomplete_word_size = new_word_size;

    printf("\33\x2\x8");
    for (uint32_t z = input_line_index - autocomplete_input_skip; z < autocomplete_word_size; z++)
        printf("%c", autocomplete_table[autocomplete_word][z]);
    printf("\33\x3");
    for (uint32_t z = input_line_index - autocomplete_input_skip; z < autocomplete_word_size; z++)
        printf("\33\x6\x1");

    return 0;
}

size_t Shell::read_input_line()
{
    memset(input_line_buffer, 0, sizeof(input_line_buffer));
    input_line_index = 0;
    autocomplete_input_skip = 0;
    autocomplete_word = -1;
    autocomplete_word_size = 0;
    is_first_word = 1;

    while (input_line_index <= sizeof(input_line_buffer) - 1) {
        flush();
        int size = getchar((int*)(input_line_buffer + input_line_index));
        if (!size)
            break;

        input_line_index++;
        int ret = handle_input_line_key();
        if (ret == 1)
            continue;
        if (ret == 2)
            break;
    }

    size_t size = strlen(input_line_buffer);
    input_line_buffer[input_line_index - 1] = 0;
    return size;
}
