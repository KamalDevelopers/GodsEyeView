#include "storage.hpp"
#include <LibC/mem.h>
#include <LibC/string.h>

VolatileStorage::VolatileStorage(uint8_t* start, uint8_t* end)
{
    mem_start = start;
    mem_end = end;
}

VolatileStorage::~VolatileStorage()
{
}

void VolatileStorage::read(uint8_t* data, uint32_t sector, uint32_t count, size_t seek)
{
    if (((uint32_t)mem_start + count + sector * 512 + seek) > (uint32_t)mem_end)
        return;
    memcpy(data, mem_start + sector * 512 + seek, count);
}

void VolatileStorage::write(uint8_t* data, uint32_t sector, uint32_t count)
{
    if (((uint32_t)mem_start + count + sector * 512) > (uint32_t)mem_end)
        return;
    memcpy(mem_start + sector * 512, data, count);
}

Storage* Storage::active = 0;
Storage::Storage()
{
    active = this;
}

Storage::~Storage()
{
}

void Storage::register_storage_device(StorageDevice* device)
{
    devices[storage_devices_count] = device;
    storage_devices_count++;
}

StorageDevice* Storage::device_at(uint32_t i)
{
    return devices[i];
}

bool Storage::is_boot_storage(StorageDevice* device)
{
    uint8_t buff[4];
    buff[0] = 0;
    buff[3] = 0;

    device->read(buff, MAGIC_SECTOR, 4);
    if (strncmp((const char*)buff, MAGIC_KERNEL, 4) == 0)
        return 1;
    return 0;
}

bool Storage::find_boot_storage()
{
    boot_storage = 0;
    for (uint32_t i = 0; i < storage_devices_count; i++) {
        if (is_boot_storage(device_at(i))) {
            boot_storage = device_at(i);
            break;
        }
    }

    return (boot_storage != 0);
}
