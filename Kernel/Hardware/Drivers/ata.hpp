#ifndef ATA_HPP
#define ATA_HPP

#include "../../mutex.hpp"
#include "../port.hpp"
#include <LibC/stdio.hpp>
#include <LibC/string.hpp>
#include <LibC/types.hpp>

class AdvancedTechnologyAttachment {

protected:
    Port16Bit dataPort;
    Port8Bit errorPort;
    Port8Bit sectorCountPort;
    Port8Bit lbaLowPort;
    Port8Bit lbaMidPort;
    Port8Bit lbaHiPort;
    Port8Bit devicePort;
    Port8Bit commandPort;
    Port8Bit controlPort;

    bool master;

public:
    AdvancedTechnologyAttachment(bool master, uint16_t port_base);
    ~AdvancedTechnologyAttachment();

    void identify();
    uint8_t* read28(uint32_t sector_num, uint8_t* data, int count = 512);
    void write28(uint32_t sector_num, uint8_t* data, uint32_t count);
    void flush();
};

#endif
