#ifndef DNS_HPP
#define DNS_HPP

#include <LibC/types.h>

typedef struct dns_entry {
    char host[256];
    uint32_t remote_ip;
} dns_entry_t;

typedef struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t question_count;
    uint16_t answer_count;
    uint16_t authority_count;
    uint16_t additional_count;
} __attribute__((packed)) dns_header_t;

typedef struct dns_qheader {
    uint16_t aname;
    uint16_t atype;
    uint16_t aclass;
    uint32_t attl;
    uint16_t alen;
} __attribute__((packed)) dns_qheader_t;

namespace DNS {
uint32_t get_host_ip(const char* host);
void query_host(const char* host, uint32_t host_len);
void handle_packet(void* packet, uint32_t length);
}

#endif
