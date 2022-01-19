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

executable_t Execf::exec(uint8_t* file_data)
{
    return executable_t {};
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
}

void Loader::add(Execf* l)
{
    if (loader_num >= MAX_LOADERS)
        return;
    execfs[loader_num] = l;
    loader_num++;
}

executable_t Loader::exec(uint8_t* file_buffer, char* loader_name)
{
    for (int i = 0; i < loader_num; i++) {
        if (loader_name) {
            if (strcmp(loader_name, execfs[i]->name()) == 0)
                return execfs[i]->exec(file_buffer);
        } else if (execfs[i]->probe(file_buffer) == 1) {
            return execfs[i]->exec(file_buffer);
        }
    }
    return executable_t {};
}

int Loader::probe(uint8_t* file_buffer, char* loader_name)
{
    for (int i = 0; i < loader_num; i++)
        if (strcmp(loader_name, execfs[i]->name()) == 0)
            return execfs[i]->probe(file_buffer);
    return -1;
}
