#ifndef IPV4_HPP
#define IPV4_HPP

#include "../Mem/mm.hpp"
#include "tcp.hpp"
#include "udp.hpp"
#include <LibC/types.h>

typedef struct ipv4_packet {
    uint8_t header_length : 4;
    uint8_t version : 4;
    uint8_t tos;
    uint16_t length;

    uint16_t ident;
    uint16_t flags;

    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;

    uint32_t source_ip;
    uint32_t destination_ip;
} __attribute__((packed)) ipv4_packet_t;

typedef struct ipv4_socket {
    uint8_t type;
    uint8_t listening;
    uint8_t connected;

    uint16_t remote_port;
    uint32_t remote_ip;

    udp_socket_t* udp_socket;
    tcp_socket_t* tcp_socket;
} ipv4_socket_t;

namespace IPV4 {
uint16_t calculate_checksum(uint16_t* data, uint32_t size = sizeof(ipv4_packet_t));
bool handle_packet(ipv4_packet_t* ipv4, uint32_t size);
void send_packet(uint32_t destination_ip, uint8_t protocol, uint8_t* buffer, uint32_t size);
}

#endif
