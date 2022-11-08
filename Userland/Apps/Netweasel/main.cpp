#include <LibC/network.h>
#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>

char recvbuffer[BUFSIZ];

void protocol_icmp(uint32_t ip)
{
    int err = 0;
    int fd = socket(NET_PROTOCOL_ICMP);
    err = connect(fd, ip, 0);

    uint8_t i = 0;
    while (i < 3) {
        err = send(fd, recvbuffer, 1);
        if (i != 0)
            printf("\n");

        printf("[%s] <- ping!\n", ntoa(ip));
        int size = recv(fd, recvbuffer, 1);
        if (!size)
            break;
        printf("[%s] -> pong!", ntoa(ip));
        i++;
    }

    flush();
    err = close(fd);
}

void protocol_udp(uint32_t ip, uint16_t port)
{
    int err = 0;
    int fd = socket(NET_PROTOCOL_UDP);
    err = connect(fd, ip, port);
    err = send(fd, recvbuffer, 1);

    while (true) {
        int size = recv(fd, recvbuffer, BUFSIZ);
        if (size == 1 && recvbuffer[0] == 10)
            break;

        printf("%s", recvbuffer);
        flush();
    }

    err = close(fd);
}

int main(int argc, char** argv)
{
    if (argc < 1) {
        printf("Usage: netweasel <protocol> [ip] [port]");
        return 0;
    }

    if (strcmp(argv[0], "dhcp") == 0) {
        dhcp_info_t dhcp_info;
        uint32_t args[5];
        args[0] = (uint32_t)&dhcp_info;
        int fd = socketcall(52, args);

        printf("local ip   %s\n", ntoa(dhcp_info.my_address));
        printf("gateway    %s\n", ntoa(dhcp_info.router_address));
        printf("subnet     %s\n", ntoa(dhcp_info.subnet_mask));
        printf("dns        %s", ntoa(dhcp_info.dns_address));
        return 0;
    }

    if (argc < 2) {
        printf("Error: no destination address specified");
        return 0;
    }

    uint32_t ip = aton(argv[1]);

    if (strcmp(argv[0], "icmp") == 0) {
        protocol_icmp(ip);
        return 0;
    }

    uint16_t port = atoi(argv[2]);
    memset(recvbuffer, 0, BUFSIZ);

    if (strcmp(argv[0], "udp") == 0) {
        if (argc < 3) {
            printf("Error: udp connection requires ip and port");
            return 0;
        }
        protocol_udp(ip, port);
    }

    return 0;
}
