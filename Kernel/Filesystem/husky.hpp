#ifndef HUSKY_HPP
#define HUSKY_HPP

#include "../Hardware/Drivers/ata.hpp"
#include "../Hardware/Drivers/cmos.hpp"
#include "vfs.hpp"

#define MAX_TRANSFER_SIZE 59392
#define MED_TRANSFER_SIZE 4096
#define MIN_TRANSFER_SIZE 512

#define MAX_TRANSFER_SECT 116
#define MED_TRANSFER_SECT 8
#define MIN_TRANSFER_SECT 1

typedef struct hhash {
    uint32_t h;
    uint32_t c;
    uint16_t t;
} __attribute__((packed)) hhash_t;

typedef struct block {
    uint8_t flags;
} __attribute__((packed)) block_t;

typedef struct indirect_block {
    uint32_t data_block_ptrs[1279];
} indirect_block_t; /* not used. */

typedef struct super_node {
    char magic[25];
    uint32_t nodes_count;
    uint32_t data_blocks_count;
    uint32_t data_block_size;
    uint32_t mount_size;
} __attribute__((packed)) super_node_t;

typedef struct node {
    uint8_t type; /* 0 = not in use */
    uint16_t uid;
    uint16_t gid;
    uint32_t size_in_bytes;
    uint32_t modification_time;
    uint32_t creation_time;
    uint32_t last_access_time;
    char name[255];

    hhash_t own_hash;
    hhash_t assoc_hash;

    uint32_t blocks_ptrs[100];
    uint32_t iblocks_ptrs[210];
} __attribute__((packed)) node_t;

class Husky : public Filesystem {

private:
    ATA* ata;
    uint32_t* ptrs_cache = 0;
    uint8_t* transfer_buffer;
    uint32_t mount_start_sector = 0;
    uint32_t previous_node_ptr = 0;
    uint32_t previous_block_ptr = 0;
    uint32_t disk_size = 0;
    super_node_t super_node;
    node_t root_node;
    node_t* nodes_cache;

    void hhash_str(hhash_t* hsh, const char* s, const size_t n);
    bool hhash_cmp(hhash_t* hsh1, hhash_t* hsh2);
    uint8_t find_empty_node(uint32_t* node_ptr, uint32_t* sector_ptr);
    uint8_t find_empty_block(uint32_t* block_ptr, uint32_t* sector_ptr);
    uint8_t find_node(hhash_t* hsh, int type, uint32_t* node_ptr);

    void write_node(node_t* node);
    uint8_t delete_file_data(node_t* node);
    uint32_t write_file_data_block_list(uint32_t* ptr_list, const char* written_data_ptr,
        uint32_t* total_written, uint32_t* size_to_write, uint32_t max_blocks);
    uint32_t read_file_data_block_list(uint32_t* ptr_list, uint8_t* read_data_ptr, uint32_t* seek,
        uint32_t* total_read, uint32_t* size_to_read, uint32_t max_blocks);

    int8_t write_file_data(node_t* node, const char* data, size_t size);

public:
    Husky(ATA* ata);
    ~Husky();

    int mount();
    void available_disk_size(uint32_t size);
    void create_root_directory();
    int write_file(char* pathname, uint8_t* data, size_t size);
    int read_file(char* pathname, uint8_t* data, size_t size, size_t seek = 0);
    int get_size(char* pathname);
    int get_uid(char* pathname);
    int get_gid(char* pathname);
    int find_file(char* pathname);

    /* int rename_file(char* file_name, char* new_file_name); */
    int unlink(char* pathname);
    int read_dir(char* dirname, fs_entry_t* entries, uint32_t count);
    void mkdir(char* pathname);

    void sector_swap(int sector_src, int sector_dest);
    void write_data(uint32_t sector_start, uint8_t* fdata, int count);
    void read_data(uint32_t sector_start, uint8_t* fdata, int count, size_t seek = 0);
};

#endif
