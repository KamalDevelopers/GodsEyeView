#include "dns.hpp"
#include "../Mem/mm.hpp"
#include "dhcp.hpp"
#include "udp.hpp"
#include <LibC/network.h>
#include <LibC/string.h>

dns_entry_t dns_entries[50];
uint32_t dns_entry_count = 0;

void DNS::handle_packet(void* packet, uint32_t length)
{
    dns_header_t* header = (dns_header_t*)packet;

    if (!header->answer_count) {
        klog("[DNS] no answer count");
        return;
    }

    if (!header->question_count) {
        klog("[DNS] no question count");
        return;
    }

    uint8_t* data = (uint8_t*)packet + sizeof(dns_header_t);
    uint32_t qc = ntohs(header->question_count);

    /* TODO: Support multiple question entries */
    if (qc > 1) {
        klog("[DNS] type not supported!");
        return;
    }

    char host[256];
    dns_qheader_t qheader;
    uint32_t address = 0;
    memset(host, 0, sizeof(host));

    uint16_t label_length = *data;
    uint32_t next = 0;

    while (label_length) {
        label_length = *data;
        memcpy(host + next, data + 1, label_length);
        data += label_length + 1;
        next += label_length;
        host[next] = '.';
        next++;
    }

    host[next - 1] = 0;
    data += 4;

    for (uint16_t i = 0; i < header->answer_count; i++) {
        memcpy(&qheader, data, sizeof(dns_qheader_t));
        data += sizeof(dns_qheader_t);

        if (ntohs(qheader.atype) == 1)
            break;
        data += ntohs(qheader.alen);
    }

    if (ntohs(qheader.alen) == 4)
        memcpy(&address, data, 4);

    if (dns_entry_count + 1 == 50)
        dns_entry_count = 0;

    if (!address) {
        query_host(host, strlen(host) - 1);
        return;
    }

    memcpy(dns_entries[dns_entry_count].host, host, strlen(host) - 1);
    dns_entries[dns_entry_count].remote_ip = address;
    dns_entry_count++;
}

uint32_t DNS::get_host_ip(const char* host)
{
    for (uint32_t i = 0; i < dns_entry_count; i++) {
        if (strcmp(host, dns_entries[i].host) == 0)
            return dns_entries[i].remote_ip;
    }
    return 0;
}

void DNS::query_host(const char* host, uint32_t host_len)
{
    uint32_t packet_len = sizeof(dns_header_t) + host_len + 6;
    uint8_t* packet = (uint8_t*)kmalloc(packet_len);
    memset(packet, 0, packet_len);

    dns_header_t* header = (dns_header_t*)packet;
    header->id = 0;
    header->flags = htons(0x0100);
    header->question_count = htons(1);
    header->answer_count = 0;
    header->authority_count = 0;
    header->additional_count = 0;

    uint8_t* question = packet + sizeof(dns_header_t);
    uint8_t* head = question;

    char c = 1;
    question++;
    const char* temp_host = host;

    while (c) {
        c = *temp_host++;

        if (c == '.' || c == '\0') {
            uint32_t label_length = (uint32_t)question - (uint32_t)head - 1;
            *head = label_length;
            head = question;
        }

        *question++ = c;
    }

    uint16_t query_type = htons(1);
    memcpy(question, &query_type, 2);
    question += 2;

    uint16_t query_class = htons(1);
    memcpy(question, &query_class, 2);
    question += 2;

    udp_socket_t socket;
    socket.local_ip = DHCP::ip();
    socket.remote_ip = DHCP::info()->dns_address;
    socket.local_port = htons(53);
    socket.remote_port = htons(53);
    UDP::send(&socket, packet, packet_len);
}
