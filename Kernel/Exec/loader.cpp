#include "loader.hpp"

Loader* Loader::load = 0;
Loader::Loader()
{
    load = this;
    formats = 0;
}

void Loader::add(ExecutableFormat* loadable)
{
    if (formats >= MAX_LOADERS)
        return;
    execfs[formats] = loadable;
    formats++;
}

executable_t Loader::exec(uint8_t* file_buffer, char* loader_name)
{
    for (int i = 0; i < formats; i++) {
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
    for (int i = 0; i < formats; i++)
        if (strcmp(loader_name, execfs[i]->name()) == 0)
            return execfs[i]->probe(file_buffer);
    return -1;
}
