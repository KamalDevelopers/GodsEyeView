#ifndef PIPE_HPP
#define PIPE_HPP

#include <LibC/types.hpp>

#define PIPE_SIZE 1024

typedef struct pipe {
    uint8_t* buffer;
    size_t total_size = 0;
    size_t size = 0;
} pipe_t;

namespace Pipe {
pipe_t* create();
void destroy(pipe_t* pipe);
void expand(pipe_t* pipe, size_t size);
void collapse(pipe_t* pipe, size_t size);
int read(pipe_t* pipe, uint8_t* buffer, size_t size);
int write(pipe_t* pipe, uint8_t* buffer, size_t size);
int append(pipe_t* pipe, uint8_t* buffer, size_t size);
}

#endif
