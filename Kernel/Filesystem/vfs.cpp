#include "vfs.hpp"

Filesystem::Filesystem()
{
}

Filesystem::~Filesystem()
{
}

int Filesystem::get_size(char* file_name)
{
    return 0;
}

int Filesystem::read_file(char* file_name, uint8_t* data)
{
    return 0;
}

int Filesystem::write_file(char* file_name, uint8_t* data, int data_length)
{
    return 0;
}

int Filesystem::find_file(char* file_name)
{
    return 0;
}

VirtualFilesystem* VirtualFilesystem::active = 0;
VirtualFilesystem::VirtualFilesystem()
{
    num_mounts = 0;
    current_mount = 0;
    num_open_files = 0;

    /* Descriptors:
     * 1 : stdin
     * 2 : stdout 
     * 3 : stderr */

    file_descriptors = 4;
    active = this;
}

void VirtualFilesystem::mount(Filesystem* fs)
{
    mounts[num_mounts] = fs;
    current_mount = num_mounts;
    num_mounts++;
}

int VirtualFilesystem::open(char* file_name)
{
    //for (int i = 0; i < num_open_files; i++)
    //if (strcmp(file_name, files[i].file_name) == 0)
    //return i;

    int mount = -1;
    for (int i = 0; i < num_mounts; i++)
        if (mounts[i]->find_file(file_name) != -1)
            mount = i;

    if (mount == -1)
        return -1;

    file_entry file;
    strcpy(file.file_name, file_name);
    file.mountfs = mount;
    file.descriptor = file_descriptors;
    file.size = mounts[mount]->get_size(file_name);
    files[num_open_files] = file;

    num_open_files++;
    file_descriptors++;

    return file_descriptors - 1;
}

int VirtualFilesystem::close(int descriptor)
{
    int index = search(descriptor);
    if (index == -1)
        return -1;

    for (int j = index; j < num_open_files - 1; j++)
        files[j] = files[j + 1];

    num_open_files--;
    return 0;
}

int VirtualFilesystem::search(int descriptor)
{
    for (int i = 0; i < num_open_files; i++) {
        if (files[i].descriptor == descriptor)
            return i;
    }
    return -1;
}

int VirtualFilesystem::write_file(int descriptor, uint8_t* data, int data_length)
{
    int index = search(descriptor);
    if (index == -1)
        return -1;
    return mounts[files[index].mountfs]->write_file(files[index].file_name, data, data_length);
}

int VirtualFilesystem::read_file(int descriptor, uint8_t* data)
{
    int index = search(descriptor);
    if (index == -1)
        return -1;
    return mounts[files[index].mountfs]->read_file(files[index].file_name, data);
}

int VirtualFilesystem::file_size(int descriptor)
{
    int index = search(descriptor);
    if (index == -1)
        return -1;
    return mounts[files[index].mountfs]->get_size(files[index].file_name);
}
