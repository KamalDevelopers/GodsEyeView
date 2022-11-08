#include "dns.hpp"
#include "../Mem/mm.hpp"
#include "dhcp.hpp"
#include "udp.hpp"
#include <LibC/network.h>
#include <LibC/string.h>

void DNS::handle_packet(void* packet, uint32_t length)
{
    dns_header_t* header = (dns_header_t*)packet;
    if (!header->answer_count)
        return;

    uint8_t* data = (uint8_t*)packet + sizeof(dns_header_t);
    uint32_t qc = ntohs(header->question_count);

    /* TODO: Support multiple question entries */
    if (qc > 1) {
        klog("DNS type not supported!");
        return;
    }

    char host[256];
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

    uint16_t aname = 0;
    uint16_t atype = 0;
    uint16_t aclass = 0;
    uint32_t attl = 0;
    uint16_t alen = 0;
    uint32_t address = 0;

    memcpy(&aname, data, 2);
    data += 2;
    memcpy(&atype, data, 2);
    data += 2;
    memcpy(&aclass, data, 2);
    data += 2;
    memcpy(&attl, data, 4);
    data += 4;
    memcpy(&alen, data, 2);
    data += 2;

    if (ntohs(alen) == 4)
        memcpy(&address, data, 4);

    /* TODO: Handle response */
    klog("host: %s address: %s", host, ntoa(address));
}

void DNS::query_host(const char* host)
{
    uint32_t host_len = strlen(host);
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
    kfree(packet);
}
