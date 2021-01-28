#ifndef VFS_HPP
#define VFS_HPP

#include "LibC/types.hpp"
#include "LibC/string.hpp"
#include "LibC/stdlib.hpp"
#include "LibC/stdio.hpp"

#define MAX_OPENFILES 200 
#define MAX_FILE_NAME 100
#define MAX_MOUNTS 5

typedef struct file_entry {
    int descriptor;
    int mountfs;
    char file_name[MAX_FILE_NAME]; 
    int size;
} file_entry_t;

class Filesystem {
public:
    Filesystem();
    ~Filesystem();

    virtual int GetSize(char* file_name);
    virtual int WriteFile(char* file_name, uint8_t* data, int data_length);
    virtual int ReadFile(char* file_name, uint8_t* data);
    virtual int FindFile(char* file_name);
};

class VirtualFilesystem {
private:
    Filesystem* mounts[MAX_MOUNTS];
    file_entry_t* files[MAX_OPENFILES];

    int Search(int descriptor);

    int num_open_files;
    int file_descriptors;

    int current_mount;
    int num_mounts;

public:
    VirtualFilesystem();
    ~VirtualFilesystem();

    static VirtualFilesystem* active;
    void Mount(Filesystem* fs);

    int Open(char* file_name);
    int Close(int descriptor);
    int WriteFile(int descriptor, uint8_t* data, int data_length);
    int ReadFile(int descriptor, uint8_t* data);
    int FileSize(int descriptor);
};

namespace VFS {

static int close(int descriptor) 
{
    return VirtualFilesystem::active->Close(descriptor);
}

static int open(char* file_name) 
{
    return VirtualFilesystem::active->Open(file_name);
}

static int write(int descriptor, uint8_t* data, int data_length) {
    return VirtualFilesystem::active->WriteFile(descriptor, data, data_length);
}

static int read(int descriptor, uint8_t* data)
{
    return VirtualFilesystem::active->ReadFile(descriptor, data);
}

static int size(int descriptor)
{
    return VirtualFilesystem::active->FileSize(descriptor);
}

/* TODO */
//int rename(char* file_name, char* new_file_name);
//int chmod(char* file_name, char* permissions);
//int unlink(int path, bool update = 1);
};

#endif
