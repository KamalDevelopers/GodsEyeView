#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>

int run(char* input, int* tape)
{
    int pointer = 0;
    int unmatched_brackets = 0;
    int program_size = strlen(input);

    char user_input[50];
    int user_input_index = 0;
    memset(user_input, 0, 50);

    for (int i = 0; i <= program_size; i++) {
        switch (input[i]) {
        case '>':
            if (pointer + 1 < 30000)
                pointer++;
            break;
        case '<':
            pointer--;
            break;
        case '+':
            tape[pointer]++;
            break;
        case '-':
            tape[pointer]--;
            break;
        case '.':
            putc(tape[pointer]);
            break;
        case ',':
            flush();
            if (user_input[user_input_index] == 0) {
                memset(user_input, 0, 50);
                read(0, user_input, 50);
                user_input_index = 0;
            }

            if (user_input_index >= 50)
                break;

            tape[pointer] = user_input[user_input_index];
            user_input_index++;
            break;
        case '[':
            if (tape[pointer] == 0) {
                unmatched_brackets++;
                while (input[i] != ']' || unmatched_brackets != 0) {
                    i++;

                    if (input[i] == '[') {
                        unmatched_brackets++;
                    } else if (input[i] == ']') {
                        unmatched_brackets--;
                    }
                }
            }
            break;
        case ']':
            if (tape[pointer] != 0) {
                unmatched_brackets++;
                while (input[i] != '[' || unmatched_brackets != 0) {
                    i--;

                    if (input[i] == ']') {
                        unmatched_brackets++;
                    } else if (input[i] == '[') {
                        unmatched_brackets--;
                    }
                }
            }
            break;
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
    if (!argc) {
        printf("No input file\n");
        return 0;
    }

    int file_descriptor;
    struct stat statbuffer;

    file_descriptor = open(argv[0], O_RDONLY);
    fstat(file_descriptor, &statbuffer);

    if (statbuffer.st_size == -1) {
        printf("File '%s' does not exist\n", argv[0]);
        return 0;
    }

    char* buffer = (char*)malloc(statbuffer.st_size);
    int* tape = (int*)malloc(30000 * sizeof(int));
    memset(tape, 0, 30000 * sizeof(int));

    read(file_descriptor, buffer, statbuffer.st_size);
    close(file_descriptor);

    run(buffer, tape);

    free(buffer);
    free(tape);
    return 0;
}
