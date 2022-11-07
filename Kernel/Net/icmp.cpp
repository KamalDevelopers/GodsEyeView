#include "icmp.hpp"
#include "../multitasking.hpp"
#include <LibC++/vector.hpp>

Vector<uint32_t, 25> received_pongs;

bool ICMP::has_pong(uint32_t destination_ip)
{
    for (uint32_t i = 0; i < received_pongs.size(); i++) {
        if (received_pongs.at(i) == destination_ip) {
            received_pongs.remove_at(i);
            return true;
        }
    }
    return false;
}

void ICMP::receive_ping(icmp_packet_t* packet, uint32_t from_ip)
{
    if (packet->type == 0) {
        if (received_pongs.is_full()) {
            for (uint16_t i = 0; i < received_pongs.size() - 1; i++)
                received_pongs.remove_at(received_pongs.size() - i - 1);
        }

        received_pongs.append(from_ip);
        TM->test_poll();
    }

    if (packet->type == 8) {
        packet->type = 0;
        packet->checksum = 0;
        packet->checksum = IPV4::calculate_checksum((uint16_t*)packet, sizeof(icmp_packet_t));
        IPV4::send_packet(from_ip, 0x01, (uint8_t*)packet, sizeof(icmp_packet_t));
    }
}

void ICMP::send_ping(uint32_t destination_ip)
{
    icmp_packet_t packet;
    packet.type = 8;
    packet.code = 0;
    packet.data = 0x3713;
    packet.checksum = 0;
    packet.checksum = IPV4::calculate_checksum((uint16_t*)&packet, sizeof(icmp_packet_t));
    IPV4::send_packet(destination_ip, 0x01, (uint8_t*)&packet, sizeof(icmp_packet_t));
}
