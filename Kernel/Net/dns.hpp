#ifndef DNS_HPP
#define DNS_HPP

#include <LibC/types.h>

typedef struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t question_count;
    uint16_t answer_count;
    uint16_t authority_count;
    uint16_t additional_count;
} __attribute__((packed)) dns_header_t;

namespace DNS {
void query_host(const char* host);
void handle_packet(void* packet, uint32_t length);
}

#endif
