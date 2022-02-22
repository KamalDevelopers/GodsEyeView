#ifndef VFS_HPP
#define VFS_HPP

#include "../Mem/mm.hpp"
#include "../multitasking.hpp"
#include "../pipe.hpp"
#include <LibC/path.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibC/types.hpp>

#define MAX_OPENFILES 200
#define MAX_FILE_NAME 100
#define MAX_MOUNTS 5
#define MAX_FILE_DESCRIPTORS INT_MAX - 10
#define DEV_AUDIO_FD MAX_FILE_DESCRIPTORS + 1
#define VFS VirtualFilesystem::active

#define FS_FILE 1
#define FS_PIPE 2

struct file_entry {
    int descriptor = 0;
    int mountfs = 0;
    char file_name[MAX_FILE_NAME];
    int file_position = 0;
    int size = 0;
    int type = 0;
    pipe_t pipe;
};

class Filesystem {
public:
    Filesystem() { }
    ~Filesystem() { }

    virtual int get_gid(char* file_name) { return 0; }
    virtual int get_uid(char* file_name) { return 0; }
    virtual int get_size(char* file_name) { return 0; }
    virtual int write_file(char* file_name, uint8_t* data, int size) { return 0; }
    virtual int read_file(char* file_name, uint8_t* data, int size, int seek = 0) { return 0; }
    virtual int find_file(char* file_name) { return 0; }
    virtual int read_dir(char* dirname, char** entries) { return 0; }
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

    int listdir(char* dirname, char** entries);
    int open_pipe(char* file_name, int flags);
    int open(char* file_name, int type = FS_FILE, int flags = 0);
    int close(int descriptor);
    int write(int descriptor, uint8_t* data, int size);
    int read(int descriptor, uint8_t* data, int size);
    int size(int descriptor);
    int uid(int descriptor);
    int gid(int descriptor);
};

#endif
