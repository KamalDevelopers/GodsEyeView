#ifndef VFS_HPP
#define VFS_HPP

#include "../pipe.hpp"
#include <LibC++/vector.hpp>
#include <LibC/path.h>
#include <LibC/poll.h>
#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>
#include <LibC/types.h>

#define MAX_OPENFILES 512
#define MAX_FILE_NAME 100
#define MAX_MOUNTS 5
#define MAX_FILE_DESCRIPTORS INT_MAX - 10
#define DEV_AUDIO_FD MAX_FILE_DESCRIPTORS + 1
#define DEV_MOUSE_FD MAX_FILE_DESCRIPTORS + 2
#define DEV_KEYBOARD_FD MAX_FILE_DESCRIPTORS + 3
#define DEV_DISPLAY_FD MAX_FILE_DESCRIPTORS + 4
#define DEV_KLOG_FD MAX_FILE_DESCRIPTORS + 5
#define VFS VirtualFilesystem::active

#define FS_TYPE_FILE 1
#define FS_TYPE_FIFO 2

class Filesystem {
public:
    Filesystem() { }
    ~Filesystem() { }

    virtual int unlink(char* file_name) { return 0; }
    virtual int stat(char* file_name, struct stat* statbuf) { return 0; }
    virtual int write_file(char* file_name, uint8_t* data, size_t size) { return 0; }
    virtual int read_file(char* file_name, uint8_t* data, size_t size, size_t seek = 0) { return 0; }
    virtual int read_dir(char* dirname, fs_entry_t* entries, uint32_t count) { return 0; }
    virtual int find_file(char* file_name) { return 0; }
};

typedef struct file_entry {
    int descriptor = 0;
    int mountfs = 0;
    char file_name[MAX_FILE_NAME];
    int file_position = 0;
    int size = 0;
    int type = 0;
    int flags = 0;
    pipe_t* pipe;
} file_entry_t;

typedef struct file_table {
    Vector<file_entry_t, MAX_OPENFILES> files;
    int descriptor_index = 4;
} file_table_t;

class VirtualFilesystem {
private:
    Filesystem* mounts[MAX_MOUNTS];
    file_table kernel_file_table;

    int search(int descriptor);

    int current_mount;
    int num_mounts;

    file_table* ft();

public:
    VirtualFilesystem();
    ~VirtualFilesystem();

    static VirtualFilesystem* active;
    void mount(Filesystem* fs);

    bool is_virtual_directory(char* dirname);
    int list_directory(char* dirname, fs_entry_t* entries, uint32_t count);
    int open_fifo(char* file_name, int flags = 0);
    int open(char* file_name, int flags = 0);
    int close_fifo(int index);
    int close(int descriptor);
    int write(int descriptor, uint8_t* data, size_t size);
    int unlink(int descriptor);
    int read(int descriptor, uint8_t* data, size_t size);
    int size(int descriptor);
    int stat(int descriptor, struct stat* statbuf);
};

#endif
