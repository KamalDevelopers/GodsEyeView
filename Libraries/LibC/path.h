#ifndef PATH_H
#define PATH_H

#include "types.h"

#define MAX_PATH_SIZE 500
#define FS_ENTRY_FILE 1
#define FS_ENTRY_DIR 2
#define FS_ENTRY_FIFO 3
#define FS_ENTRY_DEV 4
#define FS_ENTRY_VIRTDIR 5

#ifdef __cplusplus
extern "C" {
#endif

int path_resolver(char* input, bool is_dir);

#ifdef __cplusplus
}
#endif

#endif
