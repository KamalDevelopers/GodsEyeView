#include "tcp.hpp"
#include "../Tasks/multitasking.hpp"
#include "dhcp.hpp"
#include "ipv4.hpp"
#include <LibC++/vector.hpp>

uint32_t tcp_port = 32880;
Vector<tcp_socket_t*, MAX_TCP_SOCKETS, true> tcp_sockets;

int socket_ack_received(tcp_socket_t* socket, void* packet, uint16_t size, uint32_t from_ip)
{
    tcp_header_t* header = (tcp_header_t*)packet;
    if (socket->state == SYN_RECEIVED) {
        socket->state = ESTABLISHED;
        return 2;
    }

    if (socket->state == FIN_WAIT1) {
        socket->state = FIN_WAIT2;
        return -1;
    }

    if (socket->state == CLOSE_WAIT) {
        socket->state = CLOSED;
        return 0;
    }

    if (size) {
        Pipe::append(socket->receive_pipe, ((uint8_t*)packet) + header->header_size * 4, size);
        TM->test_poll();
        return 1;
    }

    return 3;
}

int socket_fin_received(tcp_socket_t* socket, void* packet, uint32_t from_ip)
{
    if (socket->state == ESTABLISHED) {
        socket->state = CLOSE_WAIT;
        socket->acknowledgement_number++;
        TCP::send(socket, 0, 0, FIN_FLAG | ACK_FLAG);
        return 0;
    }

    if (socket->state == CLOSE_WAIT) {
        socket->state = CLOSED;
        return 0;
    }

    if (socket->state == FIN_WAIT1 || socket->state == FIN_WAIT2) {
        socket->state = CLOSED;
        socket->sequence_number++;
        socket->acknowledgement_number++;
        TCP::send(socket, 0, 0, ACK_FLAG);
        return 0;
    }

    return 1;
}

int socket_syn_received(tcp_socket_t* socket, void* packet, uint32_t from_ip)
{
    if (socket->state != LISTEN)
        return 1;

    tcp_header_t* header = (tcp_header_t*)packet;
    socket->state = SYN_RECEIVED;
    socket->remote_port = header->source_port;
    socket->remote_ip = DHCP::ip();
    socket->acknowledgement_number = ntohl(header->sequence_number) + 1;
    socket->sequence_number = ntohl(header->acknowledgement_number);
    TCP::send(socket, 0, 0, SYN_FLAG | ACK_FLAG);
    return 0;
}

int socket_syn_ack_received(tcp_socket_t* socket, void* packet, uint32_t from_ip)
{
    if (socket->state != SYN_SENT)
        return 1;

    tcp_header_t* header = (tcp_header_t*)packet;
    socket->state = ESTABLISHED;
    socket->acknowledgement_number = ntohl(header->sequence_number) + 1;
    socket->sequence_number++;
    TCP::send(socket, 0, 0, ACK_FLAG);
    return 0;
}

void TCP::receive(void* packet, uint16_t length, uint32_t from_ip)
{
    tcp_header_t* header = (tcp_header_t*)packet;
    uint32_t size = length - header->header_size * 4;

    int socket_index = -1;
    for (uint32_t i = 0; i < tcp_sockets.size(); i++) {
        if ((tcp_sockets.at(i)->remote_port == header->source_port)
            && (tcp_sockets.at(i)->local_port == header->remote_port)
            && (tcp_sockets.at(i)->remote_ip == from_ip)) {
            socket_index = i;
        }
    }

    if (socket_index == -1)
        return;

    tcp_socket_t* socket = tcp_sockets.at(socket_index);
    if (header->flags & RST_SENT)
        socket->state = CLOSED;

    int reset = 0;
    int cont = 0;

    switch (header->flags) {
    case SYN_SENT | ACK_SENT:
        reset = socket_syn_ack_received(socket, packet, from_ip);
        break;

    case SYN_SENT | FIN_SENT:
    case SYN_SENT | FIN_SENT | ACK_SENT:
        reset = 1;
        break;

    case FIN_SENT | PSH_SENT | ACK_SENT:
        cont = socket_ack_received(socket, packet, size, from_ip);
        if (socket->state == CLOSED)
            return;

        socket->acknowledgement_number = ntohl(header->sequence_number) + size;
        socket->sequence_number = ntohl(header->acknowledgement_number);
        socket->state = FIN_WAIT1;
        send(socket, 0, 0, ACK_FLAG);
        return;

    case FIN_SENT:
    case FIN_SENT | ACK_SENT:
        reset = socket_fin_received(socket, packet, from_ip);
        break;

    case PSH_SENT | ACK_SENT:
        cont = socket_ack_received(socket, packet, size, from_ip);
        socket->acknowledgement_number += size;
        socket->sequence_number = ntohl(header->acknowledgement_number);
        break;

    case ACK_SENT:
        cont = socket_ack_received(socket, packet, size, from_ip);
        break;

    case SYN_SENT:
        reset = socket_syn_received(socket, packet, from_ip);
        break;

    default:
        cont = 0;
        break;
    }

    if (cont == -1)
        return;

    if (cont == 1) {
        reset = 0;
        socket->acknowledgement_number = ntohl(header->sequence_number) + size;
        socket->sequence_number = ntohl(header->acknowledgement_number);
        send(socket, 0, 0, ACK_FLAG);
    }

    if (cont == 2)
        socket->acknowledgement_number = ntohl(header->acknowledgement_number);

    if (reset)
        send(socket, 0, 0, RST_FLAG);

    if (socket->state == CLOSED)
        TCP::close(socket);
}

void TCP::connect(tcp_socket_t* socket, uint32_t ip, uint16_t port)
{
    socket->receive_pipe = Pipe::create();
    socket->remote_ip = ip;
    socket->remote_port = port;
    socket->local_ip = DHCP::ip();
    socket->remote_port = ((port & 0xFF00) >> 8) | ((port & 0x00FF) << 8);
    socket->local_port = ((tcp_port & 0xFF00) >> 8) | ((tcp_port & 0x00FF) << 8);
    socket->state = SYN_SENT;
    socket->sequence_number = 0;
    socket->acknowledgement_number = 0;
    tcp_port++;

    if (tcp_sockets.is_full()) {
        kdbg("TCP: Could not create TCP connection\n");
        return;
    }

    tcp_sockets.append(socket);
    send(socket, 0, 0, SYN_FLAG);
}

void TCP::close(tcp_socket_t* socket)
{
    if (socket->state == ESTABLISHED) {
        socket->state = FIN_WAIT1;
        send(socket, 0, 0, FIN_FLAG);
        return;
    }

    int socket_index = -1;
    for (uint32_t i = 0; i < tcp_sockets.size(); i++) {
        if ((tcp_sockets.at(i)->remote_port == socket->remote_port)
            && (tcp_sockets.at(i)->local_port == socket->local_port)
            && (tcp_sockets.at(i)->remote_ip == socket->remote_ip)) {
            socket_index = i;
        }
    }

    if (socket_index == -1) {
        /* kdbg("Could not close TCP connection tcp_sockets.size() = %d \n", tcp_sockets.size()); */
        return;
    }

    tcp_sockets.remove_at(socket_index);
}

void TCP::send(tcp_socket_t* socket, uint8_t* data, uint16_t size, uint16_t flags)
{
    if (!socket)
        return;

    uint32_t packet_size = size + sizeof(tcp_header_t) + sizeof(tcp_pseudo_header_t);
    uint8_t* buffer = (uint8_t*)kmalloc_non_eternal(packet_size, "TCP");
    memset(buffer, 0, packet_size);

    tcp_pseudo_header_t* pheader = (tcp_pseudo_header_t*)buffer;
    tcp_header_t* header = (tcp_header_t*)(buffer + sizeof(tcp_pseudo_header_t));

    header->header_size = sizeof(tcp_header_t) / 4;
    header->source_port = socket->local_port;
    header->remote_port = socket->remote_port;

    header->acknowledgement_number = htonl(socket->acknowledgement_number);
    header->sequence_number = htonl(socket->sequence_number);
    header->reserved = 0;
    header->flags = flags;
    header->window_size = htons(65535);
    header->urgent_ptr = 0;

    header->options = ((flags & SYN_FLAG) != 0) ? 0xb4050402 : 0;

    if (flags == ACK_FLAG && size)
        socket->await_ack = 1;
    socket->sequence_number += size;

    memcpy(buffer + sizeof(tcp_header_t) + sizeof(tcp_pseudo_header_t), data, size);

    pheader->source_ip = socket->local_ip;
    pheader->remote_ip = socket->remote_ip;
    pheader->protocol = 0x0600;
    pheader->length = (((size + sizeof(tcp_header_t)) & 0x00FF) << 8) | (((size + sizeof(tcp_header_t)) & 0xFF00) >> 8);

    header->checksum = 0;
    header->checksum = IPV4::calculate_checksum((uint16_t*)buffer, packet_size);

    IPV4::send_packet(socket->remote_ip, 0x06, (uint8_t*)(buffer + sizeof(tcp_pseudo_header_t)), size + sizeof(tcp_header_t));
    kfree(buffer);
}
