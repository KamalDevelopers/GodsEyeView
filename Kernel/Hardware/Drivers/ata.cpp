#include "ata.hpp"
#include <LibC/ctype.h>

#define MAX_TRANSFER_SIZE 59392
#define MED_TRANSFER_SIZE 4096
#define MIN_TRANSFER_SIZE 512

#define MAX_TRANSFER_SECT 116
#define MED_TRANSFER_SECT 8
#define MIN_TRANSFER_SECT 1

MUTEX(mutex_ata);
ATActrl::ATActrl(InterruptManager* interrupt_manager, device_descriptor_t device)
    : ata0m(interrupt_manager, device, 0x1F0, 1)
    , ata0s(interrupt_manager, device, 0x1F0, 0)
    , ata1m(interrupt_manager, device, 0x170, 1)
    , ata1s(interrupt_manager, device, 0x170, 0)
{
}

void ATActrl::identify_all()
{
    ata0m.identify();
    ata0s.identify();
    ata1m.identify();
    ata1s.identify();
}

void ATActrl::register_all(Storage* storage)
{
    if (ata0m.exists())
        storage->register_storage_device(&ata0m);
    if (ata0s.exists())
        storage->register_storage_device(&ata0s);
    if (ata1m.exists())
        storage->register_storage_device(&ata1m);
    if (ata1s.exists())
        storage->register_storage_device(&ata1s);
}

void ATActrl::enable_dma()
{
    ata0m.set_dma(true);
    ata0s.set_dma(true);
    ata1m.set_dma(true);
    ata1s.set_dma(true);
}

void ATActrl::disable_dma()
{
    ata0m.set_dma(false);
    ata0s.set_dma(false);
    ata1m.set_dma(false);
    ata1s.set_dma(false);
}

void ATActrl::kernel_debug_info_drive(ATA* drive, const char* name)
{
    klog("ATA: Drive %s %s %s", name,
        drive->exists() ? drive->firmware : "",
        drive->exists() ? drive->model : "");
}

void ATActrl::kernel_debug_info()
{
    kernel_debug_info_drive(&ata0m, "Primary Master");
    kernel_debug_info_drive(&ata0s, "Primary Slave");
    kernel_debug_info_drive(&ata1m, "Secondary Master");
    kernel_debug_info_drive(&ata1s, "Secondary Slave");
}

/* IDENT data structure found here: */
/* https://people.freebsd.org/~imp/asiabsdcon2015/works/d2161r5-ATAATAPI_Command_Set_-_3.pdf */
/* page 124 and onwards */
#define ATA_IDENT_DATA_MODEL_START 27
#define ATA_IDENT_DATA_MODEL_END 46
#define ATA_IDENT_DATA_FIRMW_START 23
#define ATA_IDENT_DATA_FIRMW_END 26

ATA::ATA(InterruptManager* interrupt_manager, device_descriptor_t device, uint32_t port_base, bool master)
    : InterruptHandler(interrupt_manager, interrupt_manager->get_hardware_interrupt_offset() + 14)
    , data_port(port_base)
    , error_port(port_base + 0x1)
    , sector_count_port(port_base + 0x2)
    , lba_low_port(port_base + 0x3)
    , lba_mid_port(port_base + 0x4)
    , lba_hi_port(port_base + 0x5)
    , device_port(port_base + 0x6)
    , command_port(port_base + 0x7)
    , control_port(port_base + 0x206)
{
    PCI::active->enable_busmaster(device);
    dma = false;

    transfer_buffer_size = PAGE_ALIGN(MAX_TRANSFER_SIZE);
    transfer_buffer = (uint8_t*)PMM->allocate_pages(transfer_buffer_size);

    prdt = (ata_prdt_t*)PMM->allocate_pages(PAGE_SIZE);
    memset(prdt, 0, sizeof(ata_prdt_t));
    prdt->buffer_phys = 0;
    prdt->transfer_size = 512;
    prdt->mark_end = 0x8000;

    bar4 = PCI::active->read(device.bus, device.device, device.function, 0x20);
    if (bar4 & 0x1)
        bar4 = bar4 & 0xfffffffc;

    this->device = device;
    this->master = master;
}

ATA::~ATA()
{
    PMM->free_pages((uint32_t)transfer_buffer, transfer_buffer_size);
    PMM->free_pages((uint32_t)prdt, PAGE_SIZE);
}

void ATA::set_dma(bool dma)
{
    this->dma = dma;
}

void ATA::identify_data()
{
    memset(model, 0, sizeof(model));
    char* write_field_model = model;
    memset(firmware, 0, sizeof(firmware));
    char* write_field_firmware = firmware;

    /* TODO: Support more ident fields */
    for (int i = 0; i < 256; i++) {
        uint16_t data = data_port.read();
        uint8_t bytes[2];

        bytes[0] = (data >> 8) & 0xFF;
        bytes[1] = data & 0xFF;

        if (i >= ATA_IDENT_DATA_MODEL_START && i < ATA_IDENT_DATA_MODEL_END) {
            *(write_field_model++) = bytes[0];
            *(write_field_model++) = bytes[1];
        }

        if (i >= ATA_IDENT_DATA_FIRMW_START && i < ATA_IDENT_DATA_FIRMW_END) {
            *(write_field_firmware++) = bytes[0];
            *(write_field_firmware++) = bytes[1];
        }
    }
}

bool ATA::identify()
{
    device_port.write(master ? 0xA0 : 0xB0);
    control_port.write(0);

    device_port.write(0xA0);
    uint8_t status = command_port.read();
    if (status == 0xFF)
        return 0;

    device_port.write(master ? 0xA0 : 0xB0);
    sector_count_port.write(0);
    lba_low_port.write(0);
    lba_mid_port.write(0);
    lba_hi_port.write(0);
    command_port.write(0xEC);

    status = command_port.read();
    if (status == 0x00)
        return 0;

    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = command_port.read();

    if (status & 0x01)
        return 0;

    identify_data();

    uint32_t pci_command_reg = PCI::active->read(device.bus, device.device, device.function, 0x04);
    if (!(pci_command_reg & (1 << 2))) {
        pci_command_reg |= (1 << 2);
        PCI::active->write(device.bus, device.device, device.function, 0x04, pci_command_reg);
    }

    does_exist = 1;
    return 1;
}

void ATA::read28_dma(uint32_t sector_number, uint8_t* data, uint32_t count, int sector_count)
{
    Mutex::lock(mutex_ata);

    prdt->transfer_size = (count < 512) ? 512 : count;
    prdt->buffer_phys = (uint32_t)transfer_buffer;

    outb(bar4, 0);
    device_port.write(0xE0 | (!master) << 4 | (sector_number & 0x0f000000) >> 24);
    outb(bar4 + 2, inb(bar4 + 0x02) | 0x04 | 0x02);
    outbl(bar4 + 4, (uint32_t)prdt);
    outb(bar4, 0x08);

    int dstatus = 0;
    while ((dstatus & (1 << 6)) == 0)
        dstatus = command_port.read();

    if (count > 512 && sector_count == 1)
        sector_count = count / 512;
    sector_count_port.write(sector_count);
    lba_low_port.write(sector_number & 0x000000ff);
    lba_mid_port.write((sector_number & 0x0000ff00) >> 8);
    lba_hi_port.write((sector_number & 0x00ff0000) >> 16);

    command_port.write(0xC8);

    dstatus = 0;
    while ((dstatus & (1 << 3)) == 0)
        dstatus = command_port.read();

    has_interrupt = 0;
    outb(bar4, 0x08 | 0x1);

    while (1) {
        if (!has_interrupt)
            continue;
        int status = inb(bar4 + 2);
        dstatus = command_port.read();
        if (!(status & 0x04))
            continue;
        if (!(dstatus & 0x80))
            break;
    }

    memcpy(data, transfer_buffer, count);

    Mutex::unlock(mutex_ata);
    return;
}

void ATA::read28_pio(uint32_t sector_number, uint8_t* data, uint32_t count)
{
    Mutex::lock(mutex_ata);
    uint16_t index = 0;

    if (sector_number > 0x0FFFFFFF) {
        Mutex::unlock(mutex_ata);
        return;
    }

    device_port.write((master ? 0xE0 : 0xF0) | ((sector_number & 0x0F000000) >> 24));
    error_port.write(0);
    sector_count_port.write(1);
    lba_low_port.write(sector_number & 0x000000FF);
    lba_mid_port.write((sector_number & 0x0000FF00) >> 8);
    lba_hi_port.write((sector_number & 0x00FF0000) >> 16);
    command_port.write(0x20);

    uint8_t status = command_port.read();
    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = command_port.read();

    if (status & 0x01) {
        Mutex::unlock(mutex_ata);
        return;
    }

    for (int i = 0; i < count; i += 2) {
        uint16_t wdata = data_port.read();
        index += 2;

        data[i] = wdata & 0x00FF;
        if (i + 1 < count)
            data[i + 1] = (wdata >> 8) & 0x00FF;
    }

    for (int i = count + (count % 2); i < BUFSIZ; i += 2)
        data_port.read();

    Mutex::unlock(mutex_ata);
    return;
}

void ATA::write28_pio(uint32_t sector_number, uint8_t* data, uint32_t count)
{
    if ((sector_number > 0x0FFFFFFF) || (count > BUFSIZ))
        return;

    Mutex::lock(mutex_ata);
    device_port.write((master ? 0xE0 : 0xF0) | ((sector_number & 0x0F000000) >> 24));
    error_port.write(0);
    sector_count_port.write(1);
    lba_low_port.write(sector_number & 0x000000FF);
    lba_mid_port.write((sector_number & 0x0000FF00) >> 8);
    lba_hi_port.write((sector_number & 0x00FF0000) >> 16);
    command_port.write(0x30);

    for (int i = 0; i < count; i += 2) {
        uint16_t wdata = data[i];
        if (i + 1 < count)
            wdata |= ((uint16_t)data[i + 1]) << 8;
        data_port.write(wdata);
    }

    for (int i = count + (count % 2); i < BUFSIZ; i += 2)
        data_port.write(0x0000);
    Mutex::unlock(mutex_ata);
}

void ATA::write28_dma(uint32_t sector_number, uint8_t* data, uint32_t count)
{
    Mutex::lock(mutex_ata);

    memcpy(transfer_buffer, data, count);
    prdt->transfer_size = (count < 512) ? 512 : count;
    prdt->buffer_phys = (uint32_t)transfer_buffer;

    outb(bar4, 0);
    outbl(bar4 + 4, (uint32_t)prdt);

    device_port.write(0xE0 | (!master) << 4 | (sector_number & 0x0f000000) >> 24);
    sector_count_port.write(1);
    lba_low_port.write(sector_number & 0x000000ff);
    lba_mid_port.write((sector_number & 0x0000ff00) >> 8);
    lba_hi_port.write((sector_number & 0x00ff0000) >> 16);
    command_port.write(0xCA);
    outb(bar4, 0x1);

    while (1) {
        int status = inb(bar4 + 2);
        int dstatus = command_port.read();
        if (!(status & 0x04))
            continue;
        if (!(dstatus & 0x80))
            break;
    }

    Mutex::unlock(mutex_ata);
}

void ATA::flush_pio()
{
    if (dma)
        return;

    device_port.write(master ? 0xE0 : 0xF0);
    command_port.write(0xE7);

    uint8_t status = command_port.read();
    if (status == 0x00)
        return;

    Mutex::lock(mutex_ata);
    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = command_port.read();
    Mutex::unlock(mutex_ata);

    if (status & 0x01)
        return;
}

void ATA::read28(uint32_t sector_number, uint8_t* data, uint32_t count, int sector_count)
{
    if (dma)
        return read28_dma(sector_number, data, count, sector_count);
    return read28_pio(sector_number, data, count);
}

void ATA::write28(uint32_t sector_number, uint8_t* data, uint32_t count)
{
    if (dma)
        return write28_dma(sector_number, data, count);
    write28_pio(sector_number, data, count);
}

void ATA::read(uint8_t* data, uint32_t sector, uint32_t count, size_t seek)
{
    int size = count;
    int sector_offset = 0;
    int data_index = 0;
    uint32_t tsize = MED_TRANSFER_SIZE;
    uint32_t ssize = MED_TRANSFER_SECT;

    if (count >= MAX_TRANSFER_SIZE) {
        tsize = MAX_TRANSFER_SIZE;
        ssize = MAX_TRANSFER_SECT;
    }

    if (!is_dma() || count <= MIN_TRANSFER_SIZE) {
        tsize = MIN_TRANSFER_SIZE;
        ssize = MIN_TRANSFER_SECT;
    }

    if (seek > tsize)
        sector += (seek - sector_offset) / tsize;

    for (; size > 0; size -= tsize) {
        if (size < tsize) {
            tsize = MIN_TRANSFER_SIZE;
            ssize = MIN_TRANSFER_SECT;
        }

        read28(sector + sector_offset, transfer_buffer, tsize, ssize);
        int i = (sector_offset) ? 0 : (seek % tsize);

        for (; i < tsize; i++) {
            if (data_index <= count)
                data[data_index] = transfer_buffer[i];
            data_index++;
        }
        sector_offset += ssize;
    }
}

void ATA::write(uint8_t* data, uint32_t sector, uint32_t count)
{
    uint32_t sector_offset = 0;
    for (uint32_t i = 0; i < count;) {
        uint32_t siz = ((count - i) >= 512) ? 512 : count - i;
        write28(sector + sector_offset, data + i, siz);
        sector_offset++;
        i += siz;
    }
}

void ATA::flush()
{
    if (dma)
        return;
    flush_pio();
}

uint32_t ATA::interrupt(uint32_t esp)
{
    uint8_t dstatus = control_port.read();
    uint8_t status = inb(bar4 + 2);
    outb(bar4, 0x0);
    has_interrupt = 1;
    return esp;
}
