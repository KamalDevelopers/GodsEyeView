#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "stream.hpp"
#include <LibC/path.hpp>
#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/string.hpp>
#include <LibC/types.hpp>
#include <LibC/unistd.hpp>

#define SOUND_PLAY_FILE 1
#define SOUND_REQUIRE_UPDATE 2
#define SOUND_EVENT_DONE 1
#define SOUND_EVENT_PLAYBACK 2

typedef struct stream_context {
    int unique_id = 0;
    int events_file = -1;
} stream_context_t;

typedef struct stream_event {
    int type = 0;
} stream_event_t;

typedef struct sound_request {
    int type = 0;
    int unique_id = 0;
    char sound_file[MAX_PATH_SIZE];
} sound_request_t;

int request_play_sound_file(stream_context_t* context, char* file);
int request_stream_wait_done(stream_context_t* context);
int request_stream_wait_playback(stream_context_t* context);
int request_server_update();

#endif
