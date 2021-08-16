#ifndef LOADER_HPP
#define LOADER_HPP

#include "LibC/stdio.hpp"
#include "LibC/string.hpp"

#define MAX_LOADERS 5

class Execf {
public:
    Execf(char* n);
    ~Execf();

    virtual int probe(uint8_t* file_data);
    virtual int exec(uint8_t* file_data);
    virtual char* name();
};

class Loader {
private:
    uint32_t loader_num;
    Execf* execfs[MAX_LOADERS];

public:
    Loader();
    ~Loader();

    static Loader* load;

    void add(Execf* l);
    int exec(uint8_t* file_buffer, char* loader_name = 0);
    int probe(uint8_t* file_buffer, char* loader_name);
};

#endif
