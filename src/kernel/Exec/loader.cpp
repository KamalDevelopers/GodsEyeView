#include "loader.hpp"

Execf::Execf(char* n)
{
}

Execf::~Execf()
{
}

int Execf::probe(uint8_t* file_data)
{
    return 0;
}

int Execf::exec(uint8_t* file_data, uint32_t phys_loc)
{
    return 0;
}

char* Execf::name()
{
    return 0;
}

Loader* Loader::load = 0;
Loader::Loader()
{
    load = this;
    loader_num = 0;
    location = 0x400000;
}

void Loader::add(Execf* l)
{
    if (loader_num >= MAX_LOADERS)
        return;
    execfs[loader_num] = l;
    loader_num++;
}

int Loader::exec(uint8_t* file_buffer, char* loader_name)
{
    for (int i = 0; i < loader_num; i++) {
        if (loader_name) {
            if (strcmp(loader_name, execfs[i]->name()) == 0) {
                location += 0x400000;
                return execfs[i]->exec(file_buffer, location);
            }
        } else if (execfs[i]->probe(file_buffer) == 1) {
            location += 0x400000;
            return execfs[i]->exec(file_buffer, location);
        }
    }
    return -1;
}

int Loader::probe(uint8_t* file_buffer, char* loader_name)
{
    for (int i = 0; i < loader_num; i++) {
        if (strcmp(loader_name, execfs[i]->name()) == 0) {
            return execfs[i]->probe(file_buffer);
        }
    }
    return -1;
}

uint32_t Loader::plocation()
{
    return location;
}
