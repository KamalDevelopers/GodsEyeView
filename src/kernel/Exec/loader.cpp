#include "loader.hpp"

Loader* Loader::load = 0;
Loader::Loader(loader_t** l, uint32_t n)
{
    load = this;
    loaders_l = l;
    loader_num = n;
    location = 0x400000;
}

int Loader::exec(uint8_t* file_buffer, char* loader_name)
{
    for (int i = 0; i < loader_num; i++) {
        if (loader_name) {
            if (strcmp(loader_name, loaders_l[i]->name) == 0) {
                location += 0x400000;
                return loaders_l[i]->exec(file_buffer, location);
            }
        } else if (loaders_l[i]->probe(file_buffer) == 1) {
            location += 0x400000;
            return loaders_l[i]->exec(file_buffer, location);
        }
    }
    return -1;
}

int Loader::probe(uint8_t* file_buffer, char* loader_name)
{
    for (int i = 0; i < loader_num; i++) {
        if (strcmp(loader_name, loaders_l[i]->name) == 0) {
            return loaders_l[i]->probe(file_buffer);
        }
    }
    return -1;
}

uint32_t Loader::plocation()
{
    return location;
}
