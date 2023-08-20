#ifndef TAR_HPP
#define TAR_HPP

#include "../Hardware/Drivers/ata.hpp"
#include "../Mem/mm.hpp"
#include "../tty.hpp"
#include "vfs.hpp"
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>

#define MAX_TRANSFER_SIZE 59392
#define MED_TRANSFER_SIZE 4096
#define MIN_TRANSFER_SIZE 512

#define MAX_TRANSFER_SECT 116
#define MED_TRANSFER_SECT 8
#define MIN_TRANSFER_SECT 1

#define SB_OFFSET 1024
#define MAX_FILES 200
#define MAX_DIRS 200
#define MAGIC "ustar"

struct posix_header {   /* byte offset */
    char name[100];     /*   0 */
    char mode[8];       /* 100 */
    char uid[8];        /* 108 */
    char gid[8];        /* 116 */
    char size[12];      /* 124 */
    char mtime[12];     /* 136 */
    char chksum[8];     /* 148 */
    char typeflag;      /* 156 */
    char linkname[100]; /* 157 */
    char magic[6];      /* 257 */
    char version[2];    /* 263 */
    char uname[32];     /* 265 */
    char gname[32];     /* 297 */
    char devmajor[8];   /* 329 */
    char devminor[8];   /* 337 */
    char prefix[155];   /* 345 */
                        /* 500 - zeros padding */
};

class Tar : public Filesystem {

private:
    ATA* ata;
    uint32_t tar_end;
    int dir_index;
    int file_index;
    uint8_t* transfer_buffer;
    posix_header dirs[MAX_DIRS];           // Maximum amount of directories in RAM
    posix_header files[MAX_FILES];         // Maximum amount of files in RAM
    uint32_t sector_links_file[MAX_FILES]; // Sector index of files

    int oct_bin(char* str, int size);
    int bin_oct(int decimal_num);
    int get_mode(int file_id, int utype);

public:
    Tar(ATA* ata);
    ~Tar();

    int mount();
    void update_disk(int uentry, int uentry_size);
    int write_file(char* file_name, uint8_t* data, size_t size);
    int read_file(int file_id, uint8_t* data, size_t size, size_t seek = 0);
    int read_file(char* file_name, uint8_t* data, size_t size, size_t seek = 0);
    int find_file(char* file_name);
    int stat(char* file_name, struct stat* statbuf);
    int get_size(char* file_name);
    int get_uid(char* file_name);
    int get_gid(char* file_name);

    int rename_file(char* file_name, char* new_file_name);
    int unlink(char* file_name, bool should_update);
    int unlink(char* file_name);
    int read_dir(char* dirname, fs_entry_t* entries, uint32_t count);
    int exists(char* name);
    int chmod(char* file_name, char* permissions);

    void sector_swap(int sector_src, int sector_dest);
    void write_data(uint32_t sector_start, uint8_t* fdata, int count);
    void read_data(uint32_t sector_start, uint8_t* fdata, int count, size_t seek = 0);

    posix_header* file_calculate_checksum(posix_header* header_data);
    int calculate_checksum(posix_header* header_data);
};

#endif
