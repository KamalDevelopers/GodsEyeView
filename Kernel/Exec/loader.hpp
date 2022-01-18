#ifndef LOADER_HPP
#define LOADER_HPP

#include "LibC/stdio.hpp"
#include "LibC/string.hpp"

#define MAX_LOADERS 5

typedef struct executable {
    uint32_t eip;
} executable_t;

class Execf {
public:
    Execf(char* n);
    ~Execf();

    virtual int probe(uint8_t* file_data);
    virtual executable_t exec(uint8_t* file_data);
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
    int probe(uint8_t* file_buffer, char* loader_name);
    executable_t exec(uint8_t* file_buffer, char* loader_name = 0);
};

#endif
