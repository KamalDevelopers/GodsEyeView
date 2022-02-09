#include "icmp.hpp"

void ICMP::receive_ping(icmp_packet_t* packet, uint32_t from_ip)
{
    if (packet->type == 0) {
        /* TODO: Do something with the ping response */
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
