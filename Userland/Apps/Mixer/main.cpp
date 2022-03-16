#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibC/unistd.hpp>
#include <LibSound/connection.hpp>
#include <LibSound/stream.hpp>

bool play(char* file_name)
{
    struct stat statbuffer;
    stat(file_name, &statbuffer);
    if (statbuffer.st_size == -1)
        return false;

    stream_context_t stream_context;
    request_play_sound_file(&stream_context, file_name);
    request_stream_wait_playback(&stream_context);

    printf("Playing %s ...", file_name);
    flush();

    request_stream_wait_done(&stream_context);

    printf("%s", "\b\b\b [done]");
    return true;
}

int main(int argc, char** argv)
{
    if (!argc) {
        printf("Usage: mixer <file>");
        return 0;
    }

    if (!play(argv[0]))
        printf("File '%s' does not exist", argv[0]);

    return 0;
}
