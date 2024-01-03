#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <LibC/types.h>

#define MAX_STORAGE_DEVICES 255

class StorageDevice {
public:
    StorageDevice() { }
    ~StorageDevice() { }

    virtual void read(uint8_t* data, uint32_t sector, uint32_t count, size_t seek = 0) { }
    virtual void write(uint8_t* data, uint32_t sector, uint32_t count) { }
    virtual void flush() { }
};

class VolatileStorage : public StorageDevice {
private:
    uint8_t* mem_start = 0;
    uint8_t* mem_end = 0;

public:
    VolatileStorage(uint8_t* start, uint8_t* end);
    ~VolatileStorage();

    void read(uint8_t* data, uint32_t sector, uint32_t count, size_t seek = 0);
    void write(uint8_t* data, uint32_t sector, uint32_t count);
    void flush() { }
};

class Storage {
private:
    StorageDevice* devices[MAX_STORAGE_DEVICES];
    StorageDevice* boot_storage = 0;
    uint32_t storage_devices_count = 0;

    bool is_boot_storage(StorageDevice* device);

public:
    Storage();
    ~Storage();

    static Storage* active;

    uint32_t device_count() { return storage_devices_count; }
    void register_storage_device(StorageDevice* device);
    StorageDevice* device_at(uint32_t i);
    StorageDevice* boot_device() { return boot_storage; }
    bool find_boot_storage();
};

/* Operating system magic identifier */

#define MAGIC_SECTOR 11
#define MAGIC_KERNEL "gods"

/* Volatile disk mode (RAM disk) */

#define VOLATILE_DISK false
#if VOLATILE_DISK
extern uint8_t* _binary_drive_start;
extern uint8_t* _binary_drive_end;
inline bool has_volatile_disk() { return true; }
#else
static uint8_t* _binary_drive_start = 0;
static uint8_t* _binary_drive_end = 0;
inline bool has_volatile_disk() { return false; }
#endif

#define RAM_DISK _binary_drive_start, _binary_drive_end

#endif
