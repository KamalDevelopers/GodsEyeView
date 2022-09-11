#ifndef PATH_HPP
#define PATH_HPP

#define MAX_PATH_SIZE 500
#define FS_ENTRY_FILE 1
#define FS_ENTRY_DIR 2
#define FS_ENTRY_FIFO 3
#define FS_ENTRY_DEV 4
#define FS_ENTRY_VIRTDIR 5

int path_resolver(char* input, bool is_dir = true);

#endif
