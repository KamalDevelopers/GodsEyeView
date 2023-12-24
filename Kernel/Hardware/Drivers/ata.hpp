#ifndef ATA_HPP
#define ATA_HPP

#include "../../Locks/mutex.hpp"
#include "../pci.hpp"
#include "../port.hpp"
#include <LibC/stdio.h>
#include <LibC/string.h>
#include <LibC/types.h>

typedef struct ata_prdt {
    uint32_t buffer_phys;
    uint16_t transfer_size;
    uint16_t mark_end;
} __attribute__((packed)) ata_prdt_t;

class ATA : public InterruptHandler {
    friend class ATActrl;

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

    ata_prdt_t prdt;
    device_descriptor_t device;

private:
    char model[40];
    char firmware[8];
    bool does_exist = 0;
    bool master = 0;
    uint32_t bar4 = 0;
    bool dma = 0;

    void identify_data();

public:
    ATA(InterruptManager* interrupt_manager, device_descriptor_t device, uint32_t port_base, bool master);
    ~ATA();

    static driver_identifier_t identifier() { return { 0x8086, 0x7010 }; }

    bool exists() { return does_exist; }
    bool is_dma() { return this->dma; }
    void set_dma(bool dma);
    bool identify();
    uint8_t* read28(uint32_t sector_num, uint8_t* data, int count = 512, int scount = 1);
    void write28(uint32_t sector_num, uint8_t* data, uint32_t count);
    uint8_t* read28_dma(uint32_t sector_num, uint8_t* data, int count = 512, int scount = 1);
    uint8_t* read28_pio(uint32_t sector_num, uint8_t* data, int count = 512);
    void write28_pio(uint32_t sector_num, uint8_t* data, uint32_t count);
    void write28_dma(uint32_t sector_num, uint8_t* data, uint32_t count);
    void flush_pio();
    void flush();

    virtual uint32_t interrupt(uint32_t esp);
};

class ATActrl {
private:
    ATA ata0m;
    ATA ata0s;
    ATA ata1m;
    ATA ata1s;

    ATA* boot_drive_ptr = 0;
    bool is_boot_drive(ATA* drive);

public:
    ATActrl(InterruptManager* interrupt_manager, device_descriptor_t device);
    ~ATActrl();

    ATA* primary_master() { return &ata0m; }
    ATA* primary_slave() { return &ata0s; }
    ATA* secondary_master() { return &ata1m; }
    ATA* secondary_slave() { return &ata1s; }
    ATA* boot_drive() { return boot_drive_ptr; }

    void kernel_debug_info_drive(ATA* drive, const char* name);
    void kernel_debug_info();

    void identify_all();
    void enable_dma();
    void disable_dma();
    void find_boot_drive();
};

#endif
