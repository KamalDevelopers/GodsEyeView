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

inline int aton(char* s)
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

inline char* ntoa(uint32_t ip)
{
	static char buf[16];
    memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%d.%d.%d.%d", ip & 0xFF, (ip >> 8) & 0xFF, 
        (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
	return buf;
}

inline uint16_t flip_short(uint16_t short_int)
{
    uint32_t first_byte = *((uint8_t*)(&short_int));
    uint32_t second_byte = *((uint8_t*)(&short_int) + 1);
    return (first_byte << 8) | (second_byte);
}

inline uint32_t flip_long(uint32_t long_int)
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
