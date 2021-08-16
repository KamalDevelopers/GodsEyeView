#ifndef VFS_HPP
#define VFS_HPP

#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "LibC/string.hpp"
#include "LibC/types.hpp"

#define MAX_OPENFILES 200
#define MAX_FILE_NAME 100
#define MAX_MOUNTS 5

struct file_entry {
    int descriptor;
    int mountfs;
    char file_name[MAX_FILE_NAME];
    int size;
};

class Filesystem {
public:
    Filesystem();
    ~Filesystem();

    virtual int get_size(char* file_name);
    virtual int write_file(char* file_name, uint8_t* data, int data_length);
    virtual int read_file(char* file_name, uint8_t* data);
    virtual int find_file(char* file_name);
};

class VirtualFilesystem {
private:
    Filesystem* mounts[MAX_MOUNTS];
    file_entry files[MAX_OPENFILES];

    int search(int descriptor);

    int num_open_files;
    int file_descriptors;

    int current_mount;
    int num_mounts;

public:
    VirtualFilesystem();
    ~VirtualFilesystem();

    static VirtualFilesystem* active;
    void mount(Filesystem* fs);

    int open(char* file_name);
    int close(int descriptor);
    int write_file(int descriptor, uint8_t* data, int data_length);
    int read_file(int descriptor, uint8_t* data);
    int file_size(int descriptor);
};

namespace VFS {

inline int close(int descriptor)
{
    return VirtualFilesystem::active->close(descriptor);
}

inline int open(char* file_name)
{
    return VirtualFilesystem::active->open(file_name);
}

inline int write(int descriptor, uint8_t* data, int data_length)
{
    return VirtualFilesystem::active->write_file(descriptor, data, data_length);
}

inline int read(int descriptor, uint8_t* data)
{
    return VirtualFilesystem::active->read_file(descriptor, data);
}

inline int size(int descriptor)
{
    return VirtualFilesystem::active->file_size(descriptor);
}

/* TODO */
//int rename(char* file_name, char* new_file_name);
//int chmod(char* file_name, char* permissions);
//int unlink(int path, bool update = 1);
};

#endif
