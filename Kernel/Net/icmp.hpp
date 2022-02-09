#ifndef ICMP_HPP
#define ICMP_HPP

#include "../tty.hpp"
#include "ethernet.hpp"
#include "ipv4.hpp"
#include <LibC/types.hpp>

typedef struct icmp_packet {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t data;
} __attribute__((packed)) icmp_packet_t;

namespace ICMP {
void receive_ping(icmp_packet_t* packet, uint32_t from_ip);
void send_ping(uint32_t destination_ip);
}

#endif
