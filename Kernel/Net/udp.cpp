#include "udp.hpp"
#include "../Mem/mm.hpp"
#include "../Tasks/multitasking.hpp"
#include "dhcp.hpp"
#include "dns.hpp"
#include "ethernet.hpp"
#include "ipv4.hpp"
#include <LibC++/vector.hpp>
#include <LibC/network.h>

uint32_t udp_port = 1024;
Vector<udp_socket_t*, MAX_UDP_SOCKETS, true> udp_sockets;

void UDP::connect(udp_socket_t* socket, uint32_t ip, uint16_t port)
{
    socket->receive_pipe = Pipe::create();
    socket->remote_ip = ip;
    socket->remote_port = port;
    socket->local_ip = DHCP::ip();
    socket->remote_port = ((port & 0xFF00) >> 8) | ((port & 0x00FF) << 8);
    socket->local_port = ((udp_port & 0xFF00) >> 8) | ((udp_port & 0x00FF) << 8);
    udp_port++;

    if (udp_sockets.is_full()) {
        kdbg("UDP: Could not create UDP connection\n");
        return;
    }

    udp_sockets.append(socket);
}

void UDP::close(udp_socket_t* socket)
{
    int socket_index = -1;
    for (uint32_t i = 0; i < udp_sockets.size(); i++) {
        if (udp_sockets[i] == socket)
            socket_index = i;
    }

    if (socket_index == -1) {
        kdbg("UDP: Could not close UDP connection\n");
        return;
    }

    if (socket->receive_pipe)
        Pipe::destroy(socket->receive_pipe);
    udp_sockets.remove_at(socket_index);
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
}

void UDP::receive(void* packet, uint32_t from_ip)
{
    udp_header_t* header = (udp_header_t*)packet;
    uint32_t port = ntohs(header->destination_port);

    if (port == 68) {
        DHCP::handle_packet((dhcp_packet_t*)((uint8_t*)packet + sizeof(udp_header_t)));
        return;
    }

    if (port == 53) {
        DNS::handle_packet((uint8_t*)packet + sizeof(udp_header_t), ntohs(header->length) - sizeof(udp_header_t));
        return;
    }

    int socket_index = -1;
    for (uint32_t i = 0; i < udp_sockets.size(); i++) {
        if ((udp_sockets.at(i)->remote_port == header->source_port)
            && (udp_sockets.at(i)->local_port == header->destination_port)
            && (udp_sockets.at(i)->remote_ip == from_ip)) {
            socket_index = i;
        }
    }

    if (socket_index == -1)
        return;

    uint16_t length = ntohs(header->length) - sizeof(udp_header_t);
    uint8_t* data = (uint8_t*)((uint32_t)packet + sizeof(udp_header_t));
    Pipe::append(udp_sockets.at(socket_index)->receive_pipe, data, length);
    TM->test_poll();
}
