#include "connection.hpp"

static int sound_file = -1;
static int process_streams = 0;

int request_play_sound_file(stream_context_t* context, char* file)
{
    while (sound_file == -1) {
        sound_file = open((char*)"/pipe/sound", O_RDWR | O_APPEND);
        usleep(100);
    }

    sound_request_t request;
    process_streams++;
    request.unique_id = getpid() * 100 + process_streams;
    request.type = SOUND_PLAY_FILE;

    char events_file_name[50];
    char id[10];
    memset(id, 0, sizeof(id));
    memset(events_file_name, 0, sizeof(events_file_name));
    itoa(request.unique_id, id);
    strcat(events_file_name, (char*)"/pipe/sound/events/");
    strcat(events_file_name, id);
    context->unique_id = request.unique_id;

    strcpy(request.sound_file, file);
    write(sound_file, &request, sizeof(sound_request_t));

    while (context->events_file == -1) {
        context->events_file = open(events_file_name, O_RDWR);
        usleep(100);
    }

    return request.unique_id;
}

int request_server_update()
{
    while (sound_file == -1) {
        sound_file = open((char*)"/pipe/sound", O_RDWR | O_APPEND);
        usleep(100);
    }

    sound_request_t request;
    request.type = SOUND_REQUIRE_UPDATE;
    write(sound_file, &request, sizeof(sound_request_t));
    return 0;
}

int request_stream_wait_playback(stream_context_t* context)
{
    stream_event_t event;
    while (true) {
        usleep(100);
        if (read(context->events_file, &event, sizeof(stream_event_t)) > 0) {
            if (event.type == SOUND_EVENT_PLAYBACK)
                break;
        }
    }
    return 0;
}

int request_stream_wait_done(stream_context_t* context)
{
    stream_event_t event;
    while (true) {
        request_server_update();
        usleep(1000);
        if (read(context->events_file, &event, sizeof(stream_event_t)) > 0) {
            if (event.type == SOUND_EVENT_DONE)
                break;
        }
    }
    return 0;
}
