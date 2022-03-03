#include "vfs.hpp"
#include "../Mem/mm.hpp"
#include "../multitasking.hpp"
#include "../tty.hpp"

VirtualFilesystem* VirtualFilesystem::active = 0;
VirtualFilesystem::VirtualFilesystem()
{
    num_mounts = 0;
    current_mount = 0;

    /* Descriptors:
     * 0 : stdin
     * 1 : stdout
     * 2 : stderr */

    kernel_file_table.descriptor_index = 4;
    kernel_file_table.num_open_files = 0;
    active = this;
}

file_table* VirtualFilesystem::ft()
{
    if (TM->is_active())
        return TM->file_table();
    return &kernel_file_table;
}

void VirtualFilesystem::mount(Filesystem* fs)
{
    mounts[num_mounts] = fs;
    current_mount = num_mounts;
    num_mounts++;
}

int VirtualFilesystem::open_fifo(char* file_name, int flags)
{
    for (int i = 0; i < ft()->num_open_files; i++)
        if (strcmp(file_name, ft()->files[i].file_name) == 0)
            return ft()->files[i].descriptor;

    for (int i = 0; i < kernel_file_table.num_open_files; i++) {
        if (strcmp(file_name, kernel_file_table.files[i].file_name) == 0) {
            ft()->files[ft()->num_open_files] = kernel_file_table.files[i];
            ft()->files[ft()->num_open_files].descriptor = ft()->descriptor_index;
            ft()->descriptor_index++;
            ft()->num_open_files++;
            return ft()->descriptor_index - 1;
        }
    }

    if (flags != FS_CREATE_FIFO)
        return -1;

    file_entry file;
    pipe_t* pipe = Pipe::create();
    strcpy(file.file_name, file_name);
    file.pipe = pipe;
    file.type = FS_FIFO;
    file.descriptor = kernel_file_table.descriptor_index;
    kernel_file_table.files[kernel_file_table.num_open_files] = file;

    if (kernel_file_table.descriptor_index >= MAX_FILE_DESCRIPTORS) {
        klog("We ran out of kernel descriptors while opening pipe");
        kernel_file_table.descriptor_index = 4;
    }

    kernel_file_table.num_open_files++;
    kernel_file_table.descriptor_index++;

    /* Make sure we get a copy of the pipe file entry */
    return open_fifo(file_name, 0);
}

int VirtualFilesystem::open(char* file_name, int flags)
{
    if (strcmp(file_name, "/dev/audio") == 0)
        return DEV_AUDIO_FD;

    if (strcmp(file_name, "/dev/mouse") == 0)
        return DEV_MOUSE_FD;

    if (strcmp(file_name, "/dev/keyboard") == 0)
        return DEV_KEYBOARD_FD;

    if (strcmp(file_name, "/dev/display") == 0)
        return DEV_DISPLAY_FD;

    int fifo = open_fifo(file_name, flags);
    if (fifo != -1)
        return fifo;

    char file_path[MAX_PATH_SIZE];
    memset(file_path, 0, MAX_PATH_SIZE);
    if (file_name[0] != '/')
        TM->task()->cwd(file_path);
    strcat(file_path, file_name);
    path_resolver(file_path, false);

    int mount = -1;
    for (int i = 0; i < num_mounts; i++)
        if (mounts[i]->find_file(file_path) != -1)
            mount = i;

    if (mount == -1)
        return -1;

    file_entry file;
    strcpy(file.file_name, file_path);
    file.mountfs = mount;
    file.descriptor = ft()->descriptor_index;
    file.size = mounts[mount]->get_size(file_path);
    file.type = FS_FILE;
    ft()->files[ft()->num_open_files] = file;

    if (ft()->descriptor_index >= MAX_FILE_DESCRIPTORS) {
        klog("We ran out of descriptors ft=%d", (uint32_t)ft());
        kernel_file_table.descriptor_index = 0;
    }

    ft()->num_open_files++;
    ft()->descriptor_index++;
    return file.descriptor;
}

int VirtualFilesystem::close_fifo(int index)
{
    Pipe::destroy(ft()->files[index].pipe);

    for (int i = 0; i < kernel_file_table.num_open_files; i++) {
        if (strcmp(kernel_file_table.files[i].file_name, ft()->files[index].file_name) == 0) {
            for (int j = i; j < kernel_file_table.num_open_files - 1; j++)
                kernel_file_table.files[j] = kernel_file_table.files[j + 1];
            kernel_file_table.num_open_files--;
            return i;
        }
    }

    return -1;
}

int VirtualFilesystem::close(int descriptor)
{
    if (descriptor >= MAX_FILE_DESCRIPTORS)
        return 1;

    int index = search(descriptor);
    if (index == -1)
        return -1;

    if (ft()->files[index].type == FS_FIFO) {
        close_fifo(index);
        if (ft() == &kernel_file_table)
            return 0;
    }

    for (int j = index; j < ft()->num_open_files - 1; j++)
        ft()->files[j] = ft()->files[j + 1];
    ft()->num_open_files--;
    return 0;
}

int VirtualFilesystem::search(int descriptor)
{
    for (int i = 0; i < ft()->num_open_files; i++) {
        if (ft()->files[i].descriptor == descriptor)
            return i;
    }
    return -1;
}

int VirtualFilesystem::write(int descriptor, uint8_t* data, int size)
{
    int index = search(descriptor);
    if ((index == -1) || (size <= 0))
        return -1;

    TM->test_poll();
    if (ft()->files[index].type == FS_FIFO)
        return Pipe::write(ft()->files[index].pipe, data, size);
    return mounts[ft()->files[index].mountfs]->write_file(ft()->files[index].file_name, data, size);
}

int VirtualFilesystem::read(int descriptor, uint8_t* data, int size)
{
    int index = search(descriptor);
    if (index == -1)
        return -1;

    if (ft()->files[index].type == FS_FIFO)
        return Pipe::read(ft()->files[index].pipe, data, size);

    int read_size = mounts[ft()->files[index].mountfs]->read_file(ft()->files[index].file_name, data, size, ft()->files[index].file_position);
    if (read_size > 0)
        ft()->files[index].file_position += size;
    return read_size;
}

int VirtualFilesystem::size(int descriptor)
{
    int index = search(descriptor);
    if (index == -1)
        return -1;

    if (ft()->files[index].type == FS_FIFO)
        return ft()->files[index].pipe->size;
    return mounts[ft()->files[index].mountfs]->get_size(ft()->files[index].file_name);
}

int VirtualFilesystem::uid(int descriptor)
{
    int index = search(descriptor);
    if (index == -1)
        return -1;
    return mounts[ft()->files[index].mountfs]->get_uid(ft()->files[index].file_name);
}

int VirtualFilesystem::gid(int descriptor)
{
    int index = search(descriptor);
    if (index == -1)
        return -1;
    return mounts[ft()->files[index].mountfs]->get_gid(ft()->files[index].file_name);
}

int VirtualFilesystem::listdir(char* dirname, fs_entry_t* entries, uint32_t count)
{
    path_resolver(dirname);
    int fscount = mounts[0]->read_dir(dirname, entries, count);
    return fscount;
}
