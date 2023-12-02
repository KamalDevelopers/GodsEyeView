#include <LibC/mem.h>
#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/types.h>
#include <LibC/unistd.h>

#define APPEND_SIZ 1024

typedef struct document {
    char* buffer;
    size_t filled_size;
    size_t size;
} document_t;

typedef struct editor {
    document_t doc;
    char last_key[2];
    uint8_t is_running;
} editor_t;

static editor edit;

int editor_open_file(const char* pathname)
{
    int fd = open(pathname, O_RDWR | O_CREAT);
    struct stat statbuffer;
    fstat(fd, &statbuffer);

    edit.doc.buffer = (char*)malloc(statbuffer.st_size + APPEND_SIZ);
    memset(edit.doc.buffer, 0, statbuffer.st_size + APPEND_SIZ);

    read(fd, (void*)edit.doc.buffer, statbuffer.st_size);
    edit.doc.filled_size = statbuffer.st_size;
    edit.doc.size = statbuffer.st_size + APPEND_SIZ;
    return fd;
}

void keyboard_input()
{
    int s = read(0, edit.last_key, 1);
    if (s || edit.last_key[0] == 'q') {
        edit.is_running = 0;
        return;
    }
}

void draw_document()
{
    int line = 0;
    for (int i = 0; i < edit.doc.filled_size; i++) {
        if (i == 0 || edit.doc.buffer[i - 1] == '\n') {
            line++;
            printf("%d. ", line);
            if (line < 100)
                printf(" ");
            if (line < 10)
                printf(" ");
        }
        printf("%c", edit.doc.buffer[i]);
    }
    printf("\n");
}

void line_write(const char* pathname)
{
    int fd = open(pathname, O_RDWR | O_CREAT);
    char input[4096];
    int size = 0;
    int lsize = 0;
    while (true) {
        printf("> ");
        flush();
        lsize = read(0, input + size, sizeof(input));
        if (input[size] == ':' && input[size + 1] == 'q')
            break;
        size += lsize;
        if (size >= 4096)
            break;
    }
    write(fd, input, size);
    close(fd);
}

int main(int argc, char** argv)
{
    edit.is_running = 1;
    edit.last_key[0] = 0;
    edit.last_key[1] = 0;
    if (!argc) {
        printf("Usage: led [file]\n");
        return 0;
    }

    int fd = open(argv[0], O_RDWR);
    close(fd);
    if (fd < 0) {
        printf("led line write (:q to quit)\n");
        line_write(argv[0]);
        printf("\n");
    }

    fd = editor_open_file(argv[0]);
    while (edit.is_running) {
        draw_document();
        keyboard_input();
    }

    close(fd);
    free(edit.doc.buffer);
    printf("\n");
    return 0;
}
