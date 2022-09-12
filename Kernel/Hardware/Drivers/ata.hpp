#ifndef ATA_HPP
#define ATA_HPP

#include "../../mutex.hpp"
#include "../pci.hpp"
#include "../port.hpp"
#include <LibC/stdio.h>
#include <LibC/string.h>
#include <LibC/types.h>

typedef struct prdt {
    uint32_t buffer_phys;
    uint16_t transfer_size;
    uint16_t mark_end;
} __attribute__((packed)) prdt_t;

class ATA : public InterruptHandler {
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

    prdt_t prdt;
    device_descriptor_t device;
    bool master;
    uint8_t memory[4096];
    uint32_t bar4;
    bool dma;

public:
    ATA(InterruptManager* interrupt_manager, device_descriptor_t device, uint32_t port_base, bool master);
    ~ATA();

    static driver_identifier_t identifier() { return { 0x8086, 0x7010 }; }

    void set_dma(bool dma);
    void identify();
    uint8_t* read28(uint32_t sector_num, uint8_t* data, int count = 512);
    void write28(uint32_t sector_num, uint8_t* data, uint32_t count);
    uint8_t* read28_dma(uint32_t sector_num, uint8_t* data, int count = 512);
    uint8_t* read28_pio(uint32_t sector_num, uint8_t* data, int count = 512);
    void write28_pio(uint32_t sector_num, uint8_t* data, uint32_t count);
    void write28_dma(uint32_t sector_num, uint8_t* data, uint32_t count);
    void flush_pio();
    void flush();

    virtual uint32_t interrupt(uint32_t esp);
};

#endif
