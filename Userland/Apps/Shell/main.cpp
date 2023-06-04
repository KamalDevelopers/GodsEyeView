#include "language.hpp"
#include "shell.hpp"
#include <LibC/stat.h>

int main(int argc, char** argv)
{
    Shell shell;
    Language language;
    bool is_running = true;

    if (argc) {
        int file_descriptor = open(argv[0], O_RDONLY);
        if (file_descriptor < 0) {
            printf("shell error: file not found %s\n", argv[0]);
            return 1;
        }

        struct stat statbuffer;
        fstat(file_descriptor, &statbuffer);
        char* script = (char*)malloc(sizeof(char) * statbuffer.st_size);
        read(file_descriptor, script, statbuffer.st_size);
        language.execute(script, statbuffer.st_size);
        flush();
        free(script);
        return 0;
    }

    while (is_running) {
        shell.write_prompt();
        int read_size = shell.read_input_line();
        if (!read_size)
            continue;

        language.execute(shell.input_line(), read_size);
        is_running = !(language.should_exit());
    }

    return 0;
}
