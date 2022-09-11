#include <LibC/path.hpp>
#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

int read_dir(char* name, bool root)
{
    fs_entry_t* entries = (fs_entry_t*)malloc(sizeof(fs_entry_t) * 100);
    int count = listdir(name, entries, 100);

    if (count == 0) {
        printf("Folder does not exist");
        free(entries);
        return -1;
    }

    for (uint32_t i = 0; i < count; i++) {
        if (entries[i].type == FS_ENTRY_DIR)
            printf("\33\x2\xC%s\33\x3  ", entries[i].name);
        if (entries[i].type == FS_ENTRY_VIRTDIR)
            printf("\33\x2\xE%s\33\x3  ", entries[i].name);
        if (entries[i].type == FS_ENTRY_FIFO)
            printf("\33\x2\xE%s\33\x3  ", entries[i].name);
        if (entries[i].type == FS_ENTRY_FILE)
            printf("%s  ", entries[i].name);
    }

    free(entries);
    return 0;
}

int main(int argc, char** argv)
{
    char cwd[100];
    getcwd(cwd);

    if (!argc) {
        read_dir(cwd, true);
    } else {
        strcat(cwd, argv[0]);
        read_dir(argv[0], false);
    }

    return 0;
}
