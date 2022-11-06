#ifndef UDP_HPP
#define UDP_HPP

#include <LibC/types.h>

#define IPV4_PROTOCOL_UDP 17
#define MAX_UDP_SOCKETS 200

typedef struct udp_header {
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed)) udp_header_t;

typedef struct udp_socket {
    uint16_t remote_port;
    uint32_t remote_ip;
    uint16_t local_port;
    uint32_t local_ip;
} udp_socket_t;

namespace UDP {
void connect(udp_socket_t* socket, uint32_t ip, uint16_t port);
void close(udp_socket_t* socket);
void send(udp_socket_t* socket, uint8_t* data, uint16_t size);
void receive(void* packet, uint32_t from_ip);
}

#endif
