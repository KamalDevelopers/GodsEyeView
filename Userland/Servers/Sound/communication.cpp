#include "communication.hpp"
#include <LibC/unistd.h>

static int client_communication_file = 0;
int init_communications()
{
    client_communication_file = mkfifo((char*)"/pipe/sound", O_RDWR);
    return client_communication_file;
}

bool receive_connections(SoundServer* sound_server)
{
    sound_request_t request;
    if (read(client_communication_file, (void*)&request, sizeof(sound_request_t))) {
        if (request.type == SOUND_PLAY_FILE)
            sound_server->play_file(request.sound_file, request.unique_id);
        if (request.type == SOUND_REQUIRE_UPDATE)
            sound_server->send_stream_events();
        return true;
    }
    return false;
}
