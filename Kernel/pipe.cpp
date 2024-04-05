#include "pipe.hpp"
#include "Locks/mutex.hpp"
#include "Mem/mm.hpp"

MUTEX(mutex_pipe);

pipe_t* Pipe::create()
{
    pipe_t* pipe = (pipe_t*)kmalloc(sizeof(pipe_t));
    pipe->total_size = 0;
    pipe->size = 0;
    expand(pipe, PIPE_SIZE);
    return pipe;
}

void Pipe::destroy(pipe_t* pipe)
{
    if (pipe->total_size != 0)
        kfree(pipe->buffer);
    kfree(pipe);
}

void Pipe::expand(pipe_t* pipe, size_t size)
{
    if (pipe->total_size == 0)
        pipe->buffer = (uint8_t*)kmalloc(sizeof(int8_t) * size);

    if (pipe->total_size != 0) {
        uint8_t* temp_buffer = (uint8_t*)kmalloc(sizeof(int8_t) * pipe->size);
        memcpy(temp_buffer, pipe->buffer, sizeof(int8_t) * pipe->size);
        uint8_t* temp_ptr = pipe->buffer;
        pipe->buffer = (uint8_t*)kmalloc(sizeof(int8_t) * size);
        memcpy(pipe->buffer, temp_buffer, sizeof(int8_t) * pipe->size);
        kfree(temp_buffer);
        kfree(temp_ptr);
    }

    pipe->total_size = size;
}

void Pipe::collapse(pipe_t* pipe, size_t size)
{
    if ((pipe->buffer == 0) || (pipe->total_size == 0))
        return;

    if (pipe->total_size <= size)
        return;

    kfree(pipe->buffer);
    if (size != 0)
        pipe->buffer = (uint8_t*)kmalloc(sizeof(int8_t) * size);
    pipe->total_size = size;
}

int Pipe::read(pipe_t* pipe, uint8_t* buffer, size_t size)
{
    if ((!size) || (!pipe->size))
        return 0;

    if ((size > pipe->total_size) || (size > pipe->size))
        size = pipe->size;

    Mutex::lock(mutex_pipe);
    memcpy(buffer, pipe->buffer, size);

    if (pipe->size - size == 0) {
        memset(pipe->buffer, 0, pipe->size);
    } else {
        for (int i = 0; i < pipe->size - size; i++) {
            pipe->buffer[i] = pipe->buffer[i + size];
            pipe->buffer[i + size] = 0;
        }
    }

    pipe->size = pipe->size - size;
    buffer[size] = 0;
    Mutex::unlock(mutex_pipe);
    return size;
}

int Pipe::write(pipe_t* pipe, uint8_t* buffer, size_t size)
{
    if (size <= 0)
        return -1;

    Mutex::lock(mutex_pipe);
    if (size > pipe->total_size)
        expand(pipe, size);
    if ((size < pipe->total_size) && (size >= PIPE_SIZE))
        collapse(pipe, size);

    pipe->size = size;
    memcpy(pipe->buffer, buffer, size);
    Mutex::unlock(mutex_pipe);
    return size;
}

int Pipe::append(pipe_t* pipe, uint8_t* buffer, size_t size)
{
    if (size <= 0)
        return -1;

    Mutex::lock(mutex_pipe);
    if (size + pipe->size > pipe->total_size) {
        uint8_t* temporary = (uint8_t*)kmalloc(pipe->total_size);
        int temporary_size = pipe->total_size;
        memcpy(temporary, pipe->buffer, pipe->size);
        expand(pipe, size + pipe->size);
        memcpy(pipe->buffer, temporary, temporary_size);
        kfree(temporary);
    }

    memcpy(pipe->buffer + pipe->size, buffer, size);
    pipe->size += size;
    Mutex::unlock(mutex_pipe);
    return size;
}
