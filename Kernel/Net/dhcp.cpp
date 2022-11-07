#include "dhcp.hpp"
#include "../Mem/mm.hpp"
#include "udp.hpp"
#include <LibC/network.h>
#include <LibC/stdlib.h>

static dhcp_info_t dhcp_info;

dhcp_info_t* DHCP::info()
{
    return &dhcp_info;
}

uint32_t DHCP::ip()
{
    if (!dhcp_info.my_address)
        return ((15 << 24) | (2 << 16) | (0 << 8) | 10);
    return dhcp_info.my_address;
}

uint32_t DHCP::gateway()
{
    if (!dhcp_info.router_address)
        return ((2 << 24) | (2 << 16) | (0 << 8) | 10);
    return dhcp_info.router_address;
}

uint32_t DHCP::subnet()
{
    if (!dhcp_info.subnet_mask)
        return ((0 << 24) | (255 << 16) | (255 << 8) | 255);
    return dhcp_info.subnet_mask;
}

void DHCP::set_info(dhcp_packet_t* packet)
{
    dhcp_info.my_address = packet->yiaddr;
    parse_options(packet, 1, &dhcp_info.subnet_mask, 4);
    parse_options(packet, 3, &dhcp_info.router_address, 4);
    parse_options(packet, 6, &dhcp_info.dns_address, 4);
}

void DHCP::discover()
{
    dhcp_packet_t* packet = (dhcp_packet_t*)kmalloc(sizeof(dhcp_packet_t));
    memset(packet, 0, sizeof(dhcp_packet_t));
    create_packet(packet, 1, 0x0);

    udp_socket_t socket;
    socket.local_ip = 0x0;
    socket.remote_ip = 0xffffffff;
    socket.local_port = htons(68);
    socket.remote_port = htons(67);

    UDP::send(&socket, (uint8_t*)packet, sizeof(dhcp_packet_t));
    kfree(packet);
}

void DHCP::request(uint32_t request_ip)
{
    dhcp_packet_t* packet = (dhcp_packet_t*)kmalloc(sizeof(dhcp_packet_t));
    memset(packet, 0, sizeof(dhcp_packet_t));
    create_packet(packet, 3, request_ip);

    udp_socket_t socket;
    socket.local_ip = request_ip;
    socket.remote_ip = 0xffffffff;
    socket.local_port = htons(68);
    socket.remote_port = htons(67);

    UDP::send(&socket, (uint8_t*)packet, sizeof(dhcp_packet_t));
    kfree(packet);
}

void DHCP::handle_packet(dhcp_packet_t* packet)
{
    uint8_t* options = packet->options + 4;
    uint8_t type;

    if (packet->op == 2) {
        parse_options(packet, 53, &type, 1);

        switch (type) {
        case DHCP_OFFER:
            set_info(packet);
            /* request(packet->yiaddr); */
            break;

        case DHCP_ACK:
            set_info(packet);
            break;
        }
    }
}

void DHCP::parse_options(dhcp_packet_t* packet, uint8_t type, void* buff, uint16_t size)
{
    uint8_t* options = packet->options + 4;

    while (options[0] != 0xff) {
        uint8_t len = *(options + 1);
        if (options[0] == type) {
            memcpy(buff, options + 2, (len > size) ? size : len);
            return;
        }
        options += (2 + len);
    }
}

void DHCP::create_packet(dhcp_packet_t* packet, uint8_t type, uint32_t request_ip)
{
    packet->op = DHCP_REQUEST;
    packet->htype = 0x01;
    packet->hlen = 6;
    packet->hops = 0;
    packet->xid = htonl(0x55555555);
    packet->flags = htons(0x8000);

    uint8_t* options = packet->options;
    *((uint32_t*)(options)) = htonl(0x63825363);
    options += 4;

    *(options++) = 53;
    *(options++) = 1;
    *(options++) = type;

    *(options++) = 61;
    *(options++) = 0x07;
    *(options++) = 0x01;
    options += 6;

    *(options++) = 50;
    *(options++) = 0x04;
    *((uint32_t*)(options)) = htonl(0x0a00020e);
    memcpy((uint32_t*)(options), &request_ip, 4);
    options += 4;

    *(options++) = 12;
    *(options++) = 0x09;
    memset(options, 0, 8);
    options += 8;
    *(options++) = 0x00;

    *(options++) = 55;
    *(options++) = 8;
    *(options++) = 0x1;
    *(options++) = 0x3;
    *(options++) = 0x6;
    *(options++) = 0xf;
    *(options++) = 0x2c;
    *(options++) = 0x2e;
    *(options++) = 0x2f;
    *(options++) = 0x39;
    *(options++) = 0xff;
}
