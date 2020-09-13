#ifndef ETHERFRAME_HPP
#define ETHERFRAME_HPP

#include "../Hardware/Drivers/amd79.hpp"
#include "stdlib.hpp"
#include "types.hpp"

namespace Network {
struct EtherFrameHeader {
    uint64_t dstMAC_BE : 48;
    uint64_t srcMAC_BE : 48;
    uint16_t etherType_BE;
} __attribute__((packed));

typedef uint32_t EtherFrameFooter;
class EtherFrameProvider;

class EtherFrameHandler {
protected:
    EtherFrameProvider* backend;
    uint16_t etherType_BE;

public:
    EtherFrameHandler(EtherFrameProvider* backend, uint16_t etherType);
    ~EtherFrameHandler();

    virtual bool OnEtherFrameReceived(uint8_t* etherframePayload, uint32_t size);
    void Send(uint64_t dstMAC_BE, uint8_t* etherframePayload, uint32_t size);
};

class EtherFrameProvider : public RawDataHandler {
    friend class EtherFrameHandler;

protected:
    EtherFrameHandler* handlers[65535];

public:
    EtherFrameProvider(AmdDriver* backend);
    ~EtherFrameProvider();

    bool OnRawDataReceived(uint8_t* buffer, uint32_t size);
    void Send(uint64_t dstMAC_BE, uint16_t etherType_BE, uint8_t* buffer, uint32_t size);

    uint64_t GetMACAddress();
    uint32_t GetIPAddress();
};
};

#endif
