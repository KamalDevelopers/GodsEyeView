#ifndef TCP_HPP
#define TCP_HPP

#include "../pipe.hpp"
#include <LibC/types.h>

#define MAX_TCP_SOCKETS 1024
#define FIN_SENT 1
#define SYN_SENT 2
#define RST_SENT 4
#define PSH_SENT 8
#define ACK_SENT 16
#define SYN_RECEIVED 3

#define FIN_FLAG 1
#define SYN_FLAG 2
#define RST_FLAG 4
#define ACK_FLAG 16

#define CLOSED 0
#define LISTEN 1
#define ESTABLISHED 4
#define CLOSING 7
#define FIN_WAIT1 5
#define FIN_WAIT2 6
#define TIME_WAIT 8
#define CLOSE_WAIT 9

typedef struct tcp_header {
    uint16_t source_port;
    uint16_t remote_port;
    uint32_t sequence_number;
    uint32_t acknowledgement_number;

    uint8_t reserved : 4;
    uint8_t header_size : 4;
    uint8_t flags;

    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_ptr;
    uint32_t options;
} __attribute__((packed)) tcp_header_t;

typedef struct tcp_pseudo_header {
    uint32_t source_ip;
    uint32_t remote_ip;
    uint16_t protocol;
    uint16_t length;
} __attribute__((packed)) tcp_pseudo_header_t;

typedef struct tcp_socket {
    uint16_t remote_port;
    uint32_t remote_ip;
    uint16_t local_port;
    uint32_t local_ip;
    uint16_t state;
    uint32_t sequence_number;
    uint32_t acknowledgement_number;
    pipe_t* receive_pipe;
} tcp_socket_t;

namespace TCP {
void connect(tcp_socket_t* socket, uint32_t ip, uint16_t port);
void send(tcp_socket_t* socket, uint8_t* data, uint16_t size, uint16_t flags);
void receive(void* packet, uint16_t length, uint32_t from_ip);
void close(tcp_socket_t* socket);
}

#endif
