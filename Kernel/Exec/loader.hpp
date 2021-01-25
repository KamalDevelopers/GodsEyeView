#ifndef LOADER_HPP
#define LOADER_HPP

#include "LibC/stdio.hpp"
#include "LibC/string.hpp"

#define MAX_LOADERS 5

class Execf {
public:
    Execf(char* n);
    ~Execf();

    virtual int Probe(uint8_t* file_data);
    virtual int Exec(uint8_t* file_data, uint32_t phys_loc);
    virtual char* Name();
};

class Loader {

private:
    uint32_t location;
    uint32_t loader_num;
    Execf* execfs[MAX_LOADERS];

public:
    Loader();
    ~Loader();

    static Loader* load;
    void Add(Execf* l);
    int Exec(uint8_t* file_buffer, char* loader_name = 0);
    int Probe(uint8_t* file_buffer, char* loader_name);
    uint32_t Location();
};

#endif
