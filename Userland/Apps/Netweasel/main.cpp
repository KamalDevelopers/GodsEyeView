#include <LibC/network.h>
#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>

char recvbuffer[BUFSIZ];

void protocol_udp(uint32_t ip, uint16_t port)
{
    int err = 0;
    uint32_t args[5];
    args[0] = NET_PROTOCOL_UDP;
    int fd = socketcall(1, args);

    args[0] = fd;
    args[1] = ip;
    args[2] = port;
    err = socketcall(3, args);

    args[0] = fd;
    args[1] = (uint32_t)recvbuffer;
    args[2] = 1;
    err = socketcall(9, args);

    while (true) {
        args[0] = fd;
        args[1] = (uint32_t)recvbuffer;
        args[2] = BUFSIZ;
        int size = socketcall(10, args);
        if (size == 1 && recvbuffer[0] == 10)
            break;

        printf("%s", recvbuffer);
        flush();
    }

    args[0] = fd;
    err = socketcall(13, args);
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("Usage: netweasel <protocol> <ip> <port>");
        return 0;
    }

    uint32_t ip = aton(argv[1]);
    uint16_t port = atoi(argv[2]);
    memset(recvbuffer, 0, BUFSIZ);

    if (ip != ((2 << 24) | (2 << 16) | (0 << 8) | 10))
        return 0;

    if (strcmp(argv[0], "udp") == 0)
        protocol_udp(ip, port);
    return 0;
}
