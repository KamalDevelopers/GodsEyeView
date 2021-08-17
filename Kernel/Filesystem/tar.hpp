#ifndef TAR_HPP
#define TAR_HPP

#include "../Hardware/Drivers/ata.hpp"
#include "../Mem/mm.hpp"
#include "../tty.hpp"
#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "LibC/string.hpp"
#include "vfs.hpp"

#define SB_OFFSET 1024
#define MAX_FILES 200
#define MAX_DIRS 100
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
    AdvancedTechnologyAttachment* hd;
    uint32_t tar_end;
    int dir_index;
    int file_index;
    posix_header dirs[MAX_DIRS];           // Maximum amount of directories in RAM
    posix_header files[MAX_FILES];         // Maximum amount of files in RAM
    uint32_t sector_links_dir[MAX_DIRS];   // Sector index of directories
    uint32_t sector_links_file[MAX_FILES]; // Sector index of files

    int oct_bin(char* str, int size);
    int bin_oct(int decimal_num);
    int get_mode(int file_id, int utype);

public:
    Tar(AdvancedTechnologyAttachment* ata);
    ~Tar();

    void mount();
    void update(int uentry, int uentry_size);

    int write_file(char* file_name, uint8_t* data, int data_length);
    int read_file(int file_id, uint8_t* data);
    int read_file(char* file_name, uint8_t* data);
    int find_file(char* file_name);
    int get_size(char* file_name);
    int get_uid(char* file_name);
    int get_gid(char* file_name);

    int rename_file(char* file_name, char* new_file_name);
    int unlink(char* path, bool should_update = 1);
    int read_dir(char* dirname, char** file_ids);
    int exists(char* name);
    int chmod(char* file_name, char* permissions);

    void sector_swap(int sector_src, int sector_dest);
    void write_data(uint32_t sector_start, uint8_t* fdata, int count);
    void read_data(uint32_t sector_start, uint8_t* fdata, int count);

    posix_header* file_calculate_checksum(posix_header* header_data);
    int calculate_checksum(posix_header* header_data);
};

#endif
