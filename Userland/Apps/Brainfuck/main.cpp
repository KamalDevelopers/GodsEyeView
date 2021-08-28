#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

int run(char* input)
{
    int pointer = 0;
    int tape[30000];
    int unmatched_brackets = 0;

    for (int i = 0; i < strlen(input); i++) {
        switch (input[i]) {
        case '>':
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
            printf("%c\0", tape[pointer]);
            break;
        case ',':
            //Not implemented yet.
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
    if (argc) {
        int file_descriptor;
        struct stat statbuffer;

        file_descriptor = open((char*)argv[0]);
        fstat(file_descriptor, &statbuffer);
        char* buffer = (char*)malloc(sizeof(char) * statbuffer.st_size);

        read(file_descriptor, buffer, statbuffer.st_size);
        close(file_descriptor);

        run(buffer);
        free(buffer);
    } else {
        printf("No input file\0");
    }
    return 0;
}
