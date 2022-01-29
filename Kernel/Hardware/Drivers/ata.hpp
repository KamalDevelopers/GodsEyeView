#ifndef ATA_HPP
#define ATA_HPP

#include "../../mutex.hpp"
#include "../port.hpp"
#include <LibC/stdio.hpp>
#include <LibC/string.hpp>
#include <LibC/types.hpp>

class AdvancedTechnologyAttachment {

protected:
    Port16Bit data_port;
    Port8Bit error_port;
    Port8Bit sector_count_port;
    Port8Bit lba_low_port;
    Port8Bit lba_mid_port;
    Port8Bit lba_hi_port;
    Port8Bit device_port;
    Port8Bit command_port;
    Port8Bit control_port;

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
