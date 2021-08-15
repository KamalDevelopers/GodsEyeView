#include "loader.hpp"

Execf::Execf(char* n)
{
}

Execf::~Execf()
{
}

int Execf::Probe(uint8_t* file_data)
{
    return 0;
}

int Execf::Exec(uint8_t* file_data)
{
    return 0;
}

char* Execf::Name()
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

void Loader::Add(Execf* l)
{
    if (loader_num >= MAX_LOADERS)
        return;
    execfs[loader_num] = l;
    loader_num++;
}

int Loader::Exec(uint8_t* file_buffer, char* loader_name)
{
    for (int i = 0; i < loader_num; i++) {
        if (loader_name) {
            if (strcmp(loader_name, execfs[i]->Name()) == 0) {
                /* FIXME: memory area for the program should be created */
                location += 0x40000;
                return execfs[i]->Exec(file_buffer);
            }
        } else if (execfs[i]->Probe(file_buffer) == 1) {
            location += 0x40000;
            return execfs[i]->Exec(file_buffer);
        }
    }
    return -1;
}

int Loader::Probe(uint8_t* file_buffer, char* loader_name)
{
    for (int i = 0; i < loader_num; i++) {
        if (strcmp(loader_name, execfs[i]->Name()) == 0) {
            return execfs[i]->Probe(file_buffer);
        }
    }
    return -1;
}

uint32_t Loader::Location()
{
    return location;
}
