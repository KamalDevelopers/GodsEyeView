#ifndef VFS_HPP
#define VFS_HPP

#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibC/types.hpp>

#define MAX_OPENFILES 200
#define MAX_FILE_NAME 100
#define MAX_MOUNTS 5
#define VFS VirtualFilesystem::active

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

    virtual int get_gid(char* file_name) { return 0; }
    virtual int get_uid(char* file_name) { return 0; }
    virtual int get_size(char* file_name) { return 0; }
    virtual int write_file(char* file_name, uint8_t* data, int data_length) { return 0; }
    virtual int read_file(char* file_name, uint8_t* data) { return 0; }
    virtual int find_file(char* file_name) { return 0; }
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
    int write(int descriptor, uint8_t* data, int data_length);
    int read(int descriptor, uint8_t* data);
    int size(int descriptor);
    int uid(int descriptor);
    int gid(int descriptor);
};

#endif
