#include "udp.hpp"
#include "../Mem/mm.hpp"
#include "ethernet.hpp"
#include "ipv4.hpp"

static uint32_t udp_port = 1024;

void UDP::connect(udp_socket_t* socket, uint32_t ip, uint16_t port)
{
    socket->remote_ip = ip;
    socket->remote_port = port;
    socket->local_ip = IP;
    socket->remote_port = ((port & 0xFF00) >> 8) | ((port & 0x00FF) << 8);
    socket->local_port = ((udp_port & 0xFF00) >> 8) | ((udp_port & 0x00FF) << 8);
    udp_port++;
}

void UDP::send(udp_socket_t* socket, uint8_t* data, uint16_t size)
{
    uint16_t length = size + sizeof(udp_header_t);
    uint8_t* buffer = (uint8_t*)kmalloc(length);
    memset(buffer, 0, length);

    udp_header_t* head = (udp_header_t*)buffer;

    head->source_port = socket->local_port;
    head->destination_port = socket->remote_port;
    head->length = ((length & 0x00FF) << 8) | ((length & 0xFF00) >> 8);
    head->checksum = 0;
    memcpy(buffer + sizeof(udp_header_t), data, size);

    IPV4::send_packet(socket->remote_ip, IPV4_PROTOCOL_UDP, buffer, length);

    kfree(buffer);
}

void UDP::receive(void* packet)
{
    udp_header_t* header = (udp_header_t*)packet;
    uint16_t dst_port = ntohs(header->destination_port);
    uint16_t src_port = ntohs(header->source_port);
    uint16_t length = ntohs(header->length) - sizeof(udp_header_t);
    uint8_t* data = (uint8_t*)((uint32_t)packet + sizeof(udp_header_t));

    klog("udp: %s", (char*)data);
}
