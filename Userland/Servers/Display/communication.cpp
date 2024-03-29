#include "communication.hpp"
#include <LibC/unistd.h>

static int client_communication_file = 0;
int init_communications()
{
    client_communication_file = mkfifo("/pipe/display", O_RDWR);
    return client_communication_file;
}

bool receive_connections(WindowManager* wm)
{
    display_request_t request;
    if (read(client_communication_file, (void*)&request, sizeof(display_request_t))) {
        if (request.type == DISPLAY_UPDATE)
            wm->require_update(request.pid);
        if (request.type == DISPLAY_DESTROY)
            wm->destroy_window_pid(request.pid);
        if (request.type == DISPLAY_CREATE)
            wm->create_window(request.width, request.height, request.pid, request.bg, request.flags);
        return true;
    }
    return false;
}
