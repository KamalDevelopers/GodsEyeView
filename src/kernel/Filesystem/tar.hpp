#ifndef TAR_HPP
#define TAR_HPP

#include "../Hardware/Drivers/ata.hpp"
#include "../Mem/mm.hpp"
#include "../tty.hpp"
#include "stdio.hpp"
#include "stdlib.hpp"
#include "string.hpp"

#define SB_OFFSET 1024
#define MAX_FILES 50 // Reduce to decrease RAM usage
#define MAX_DIRS 20  // Maximum amount of directories and files on disk
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

class Tar {

private:
    AdvancedTechnologyAttachment* hd;
    uint32_t tar_end;
    int dir_index;
    int file_index;
    posix_header dirs[MAX_DIRS];           // Maximum amount of directories in RAM
    posix_header files[MAX_FILES];         // Maximum amount of files in RAM
    uint32_t sector_links_dir[MAX_DIRS];   // Sector index of directories
    uint32_t sector_links_file[MAX_FILES]; // Sector index of files

    int OctBin(char* str, int size);
    int BinOct(int decimal_num);
    int GetMode(int file_id, int utype);

public:
    Tar(AdvancedTechnologyAttachment* ata);
    ~Tar();

    void Mount();
    void Update(int uentry, int uentry_size);

    int WriteFile(char* file_name, uint8_t* data, int data_length);
    int ReadFile(int file_id, uint8_t* data);
    int ReadFile(char* file_name, uint8_t* data);
    int FindFile(char* file_name);
    int GetSize(char* file_name);
    int RenameFile(char* file_name, char* new_file_name);
    int Unlink(char* path, bool update = 1);
    int ReadDir(char* dirname, char** file_ids);
    int Chmod(char* file_name, char* permissions);

    void SectorSwap(int sector_src, int sector_dest);
    void WriteData(uint32_t sector_start, uint8_t* fdata, int count);
    void ReadData(uint32_t sector_start, uint8_t* fdata, int count);

    posix_header* FileCalculateChecksum(posix_header* header_data);
    int CalculateChecksum(posix_header* header_data);
};

#endif
