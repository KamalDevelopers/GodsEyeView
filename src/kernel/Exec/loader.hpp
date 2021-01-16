#ifndef LOADER_HPP
#define LOADER_HPP

#include "stdlib.hpp"

typedef struct {
    char* name;
    int (*probe)(uint8_t* file_data);
    int (*exec)(uint8_t* file_data, uint32_t phys_loc);
} loader_t;

class Loader {

private:
    uint32_t location;
    uint32_t loader_num;
    loader_t** loaders_l;

public:
    static Loader* load;

    Loader(loader_t** l, uint32_t n);

    int exec(uint8_t* file_buffer, char* loader_name = 0);
    int probe(uint8_t* file_buffer, char* loader_name);
    uint32_t plocation();
};

#endif
