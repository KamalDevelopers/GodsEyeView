#ifndef IPV4_HPP
#define IPV4_HPP

#include "arp.hpp"
#include "etherframe.hpp"
#include "types.hpp"

namespace Network {
struct InternetProtocolV4Message {
    uint8_t headerLength : 4;
    uint8_t version : 4;
    uint8_t tos;
    uint16_t totalLength;

    uint16_t ident;
    uint16_t flagsAndOffset;

    uint8_t timeToLive;
    uint8_t protocol;
    uint16_t checksum;

    uint32_t srcIP;
    uint32_t dstIP;
} __attribute__((packed));

class InternetProtocolProvider;

class InternetProtocolHandler {
protected:
    InternetProtocolProvider* backend;
    uint8_t ip_protocol;

public:
    InternetProtocolHandler(InternetProtocolProvider* backend, uint8_t protocol);
    ~InternetProtocolHandler();

    virtual bool OnInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE,
        uint8_t* internetprotocolPayload, uint32_t size);
    void Send(uint32_t dstIP_BE, uint8_t* internetprotocolPayload, uint32_t size);
};

class InternetProtocolProvider : EtherFrameHandler {
    friend class InternetProtocolHandler;

protected:
    InternetProtocolHandler* handlers[255];
    AddressResolutionProtocol* arp;
    uint32_t gatewayIP;
    uint32_t subnetMask;

public:
    InternetProtocolProvider(EtherFrameProvider* backend,
        AddressResolutionProtocol* arp,
        uint32_t gatewayIP, uint32_t subnetMask);
    ~InternetProtocolProvider();

    bool OnEtherFrameReceived(uint8_t* etherframePayload, uint32_t size);
    void Send(uint32_t dstIP_BE, uint8_t protocol, uint8_t* buffer, uint32_t size);
    static uint16_t Checksum(uint16_t* data, uint32_t lengthInBytes);
};
};

#endif
