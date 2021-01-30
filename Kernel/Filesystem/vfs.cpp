#include "vfs.hpp"

Filesystem::Filesystem()
{
}

Filesystem::~Filesystem()
{
}

int Filesystem::GetSize(char* file_name)
{
    return 0;
}

int Filesystem::ReadFile(char* file_name, uint8_t* data)
{
    return 0;
}

int Filesystem::WriteFile(char* file_name, uint8_t* data, int data_length)
{
    return 0;
}

int Filesystem::FindFile(char* file_name)
{
    return 0;
}

VirtualFilesystem* VirtualFilesystem::active = 0;
VirtualFilesystem::VirtualFilesystem()
{
    num_mounts = 0;
    current_mount = 0;

    /* Descriptors:
     * 1 : stdin
     * 2 : stdout 
     * 3 : stderr */
    num_open_files = 0;
    file_descriptors = 4;
    active = this;
}

void VirtualFilesystem::Mount(Filesystem* fs)
{
    mounts[num_mounts] = fs;
    current_mount = num_mounts;
    num_mounts++;
}

int VirtualFilesystem::Open(char* file_name)
{
    for (int i = 0; i < num_open_files; i++)
        if (strcmp(file_name, files[i]->file_name) == 0)
            return i;

    int mount = -1;
    for (int i = 0; i < num_mounts; i++)
        if (mounts[i]->FindFile(file_name) != -1)
            mount = i;

    if (mount == -1)
        return -1;

    file_entry_t* file;
    strcpy(file->file_name, file_name);
    file->mountfs = mount;
    file->descriptor = file_descriptors;
    file->size = mounts[mount]->GetSize(file_name);
    files[num_open_files] = file;

    num_open_files++;
    file_descriptors++;

    return file_descriptors - 1;
}

int VirtualFilesystem::Close(int descriptor)
{
    int index = -1;
    for (int i = 0; i < num_open_files; i++)
        if (files[i]->descriptor == descriptor)
            index = i;
    if (index == -1)
        return -1;

    deleteElement(index, num_open_files, files);
    num_open_files--;
    return 0;
}

int VirtualFilesystem::Search(int descriptor)
{
    for (int i = 0; i < num_open_files; i++)
        if (files[i]->descriptor == descriptor)
            return i;
    return -1;
}

int VirtualFilesystem::WriteFile(int descriptor, uint8_t* data, int data_length)
{
    int index = Search(descriptor);
    if (index == -1)
        return -1;
    return mounts[files[index]->mountfs]->WriteFile(files[index]->file_name, data, data_length);
}

int VirtualFilesystem::ReadFile(int descriptor, uint8_t* data)
{
    int index = Search(descriptor);
    if (index == -1)
        return -1;
    return mounts[files[index]->mountfs]->ReadFile(files[index]->file_name, data);
}

int VirtualFilesystem::FileSize(int descriptor)
{
    int index = Search(descriptor);
    if (index == -1)
        return -1;
    return mounts[files[index]->mountfs]->GetSize(files[index]->file_name);
}
