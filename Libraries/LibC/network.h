#ifndef NETWORK_H
#define NETWORK_H 

#include "unistd.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#define NET_PROTOCOL_ICMP 1
#define NET_PROTOCOL_UDP 5

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dhcp_info {
    uint32_t subnet_mask;
    uint32_t router_address;
    uint32_t dns_address;
    uint32_t my_address;
} dhcp_info_t;

static uint32_t network_args[5];

inline int socket(int type)
{
    network_args[0] = type;
    return socketcall(1, network_args);
}

inline int connect(int fd, uint32_t remote_ip, uint16_t remote_port)
{
    network_args[0] = fd;
    network_args[1] = remote_ip;
    network_args[2] = remote_port;
    return socketcall(3, network_args);
}

inline int send(int fd, void* buffer, uint32_t len)
{
    network_args[0] = fd;
    network_args[1] = (uint32_t)buffer;
    network_args[2] = len;
    return socketcall(9, network_args);
}

inline int recv(int fd, void* buffer, uint32_t len)
{
    network_args[0] = fd;
    network_args[1] = (uint32_t)buffer;
    network_args[2] = len;
    return socketcall(10, network_args);
}

inline int close(int fd)
{
    network_args[0] = fd;
    return socketcall(13, network_args);
}

static int aton(char* s)
{
    uint8_t i = 0;
    uint8_t d[4] = { 0, 0, 0, 0 };
    char* tok = strtok(s, ".");

    while (tok && i < 4) {
        d[i] = atoi(tok);
        tok = strtok(NULL, ".");
        i++;
    }

    return ((d[3] << 24) | (d[2] << 16) | (d[1] << 8) | d[0]);
}

static char* ntoa(uint32_t ip)
{
    static char buf[16];
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d", ip & 0xFF, (ip >> 8) & 0xFF, 
        (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
    return buf;
}

static uint16_t flip_short(uint16_t short_int)
{
    uint32_t first_byte = *((uint8_t*)(&short_int));
    uint32_t second_byte = *((uint8_t*)(&short_int) + 1);
    return (first_byte << 8) | (second_byte);
}

static uint32_t flip_long(uint32_t long_int)
{
    uint32_t first_byte = *((uint8_t*)(&long_int));
    uint32_t second_byte = *((uint8_t*)(&long_int) + 1);
    uint32_t third_byte = *((uint8_t*)(&long_int)  + 2);
    uint32_t fourth_byte = *((uint8_t*)(&long_int) + 3);
    return (first_byte << 24) | (second_byte << 16) | (third_byte << 8) | (fourth_byte);
}

inline uint8_t flip_byte(uint8_t byte, int num_bits)
{
    uint8_t t = byte << (8 - num_bits);
    return t | (byte >> num_bits);
}

inline uint8_t htonb(uint8_t byte, int num_bits)
{
    return flip_byte(byte, num_bits);
}

inline uint8_t ntohb(uint8_t byte, int num_bits)
{
    return flip_byte(byte, 8 - num_bits);
}

inline uint16_t htons(uint16_t hostshort)
{
    return flip_short(hostshort);
}

inline uint32_t htonl(uint32_t hostlong)
{
    return flip_long(hostlong);
}

inline uint16_t ntohs(uint16_t netshort)
{
    return flip_short(netshort);
}

inline uint32_t ntohl(uint32_t netlong)
{
    return flip_long(netlong);
}

#ifdef __cplusplus
}
#endif

#endif
