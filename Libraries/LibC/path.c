#include "path.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

int path_resolver(char* input, bool is_dir)
{
    char path_buffer[MAX_PATH_SIZE];

    if (strlen(input) > MAX_PATH_SIZE)
        return -1;

    memset(path_buffer, 0, MAX_PATH_SIZE);
    strcpy(path_buffer, input);
    memset(input, 0, strlen(input));

    if ((is_dir) && (path_buffer[strlen(path_buffer)] != '/'))
        strcat(path_buffer, "/");

    char* token = strtok(path_buffer, "/");
    int last_token_size = 0;

    /* NOTE: remove this line for tar support */
    strcat(input, "/");

    while (token != NULL) {
        char* next_token = strtok(NULL, "/");

        if (strcmp(token, "..") == 0) {
            int size = strlen(input);
            input[size - last_token_size - 1] = 0;
        }

        else if (strcmp(token, ".") == 0) {
        }

        else {
            strcat(input, token);
            if ((next_token != NULL) || is_dir)
                strcat(input, "/");
        }

        last_token_size = strlen(token);
        token = next_token;
    }

    return 0;
}
