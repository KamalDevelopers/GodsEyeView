#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>
#include <LibC/unistd.h>
#include <LibSound/connection.hpp>
#include <LibSound/stream.hpp>

bool play(char* file_name)
{
    static char pathname[BUFSIZ];
    memset(pathname, 0, sizeof(pathname));
    getcwd(pathname);
    strcat(pathname, file_name);
    path_resolver(pathname, 0);

    struct stat statbuffer;
    stat(pathname, &statbuffer);
    if (statbuffer.st_size == -1)
        return false;

    stream_context_t stream_context;
    request_play_sound_file(&stream_context, pathname);
    request_stream_wait_playback(&stream_context);

    printf("Playing %s ...", file_name);
    flush();

    request_stream_wait_done(&stream_context);

    printf("%s", "\b\b\b [done]\n");
    return true;
}

int main(int argc, char** argv)
{
    if (!argc) {
        printf("Usage: mixer <file>\n");
        return 0;
    }

    if (!play(argv[0]))
        printf("File '%s' does not exist\n", argv[0]);

    return 0;
}
