#ifndef LOADER_HPP
#define LOADER_HPP

#include <LibC/stdio.h>
#include <LibC/string.h>

#define MAX_LOADERS 5
#define BINARY_MAX_SECTIONS 10

typedef struct memory_region {
    uint32_t virtual_address = 0;
    uint32_t physical_address = 0;
    uint32_t size = 0;
} memory_region_t;

typedef struct executable {
    bool valid = false;
    uint32_t eip = 0;
    uint8_t* program_data = 0;
    memory_region_t memory;
} executable_t;

class ExecutableFormat {
public:
    ExecutableFormat(char* n) { }
    ~ExecutableFormat() { }

    virtual int probe(uint8_t* file_data) { return {}; }
    virtual executable_t exec(uint8_t* file_data) { return {}; }
    virtual char* name() { return {}; }
};

class Loader {
private:
    uint32_t formats = 0;
    ExecutableFormat* execfs[MAX_LOADERS];

public:
    Loader();
    ~Loader();

    static Loader* load;

    void add(ExecutableFormat* loadable);
    int probe(uint8_t* file_buffer, char* loader_name);
    executable_t exec(uint8_t* file_buffer, char* loader_name = 0);
};

#endif
