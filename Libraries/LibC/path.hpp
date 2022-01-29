#ifndef PATH_HPP
#define PATH_HPP

#define MAX_PATH_SIZE 500

bool is_dir(char* name);
int path_resolver(char* input, bool is_dir = true);

#endif
