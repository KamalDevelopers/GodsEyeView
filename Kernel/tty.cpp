#include "tty.hpp"
#include "multitasking.hpp"

TTY::TTY()
{
    pipe_stdout = Pipe::create();
    pipe_stdin = Pipe::create();
}

TTY::~TTY()
{
    Pipe::destroy(pipe_stdout);
    Pipe::destroy(pipe_stdin);
}

int TTY::write_stdout(uint8_t* buffer, uint32_t size)
{
    return Pipe::append(pipe_stdout, buffer, size);
}

int TTY::read_stdout(uint8_t* buffer, uint32_t size)
{
    memset(buffer, 0, size);
    return Pipe::read(pipe_stdout, buffer, size);
}

int TTY::write_stdin(uint8_t* buffer, uint32_t size)
{
    /* FIXME: Properly handle backspace characters */
    if (!size)
        return 0;

    if (stdin_raw_mode)
        return Pipe::append(pipe_stdin, buffer, size);

    bool is_backspace = (buffer[size - 1] == '\b');
    if (read_stdin_size <= 0)
        return 0;

    if ((is_backspace && can_backspace()) || !is_backspace) {
        write_stdout(buffer, size);
        stdin_keypress_size += is_backspace ? -1 : 1;

        if (is_backspace) {
            if (!pipe_stdin->size)
                return 0;
            pipe_stdin->buffer[pipe_stdin->size - 1] = 0;
            pipe_stdin->size--;
        } else {
            return Pipe::append(pipe_stdin, buffer, size);
        }
    }

    return 0;
}

int TTY::read_stdin(uint8_t* buffer, uint32_t size)
{
    memset(buffer, 0, size);
    return Pipe::read(pipe_stdin, buffer, size);
}

void TTY::set_raw(bool mode)
{
    stdin_raw_mode = mode;
}

bool TTY::should_wake_stdin()
{
    if (pipe_stdin->size == 0)
        return false;
    return ((pipe_stdin->size >= read_stdin_size)
        || (pipe_stdin->buffer[pipe_stdin->size - 1] == 10));
}

int TTY::task_read_stdin(uint8_t* buffer, uint32_t size)
{
    read_stdin_size = size;
    TM->task()->sleep(-SLEEP_WAIT_STDIN);
    TM->yield();
    read_stdin_size = 0;
    stdin_keypress_size = 0;
    return Pipe::read(pipe_stdin, buffer, size);
}

int TTY::task_getchar(int* character)
{
    set_raw(true);
    read_stdin_size = 1;
    TM->task()->sleep(-SLEEP_WAIT_STDIN);
    TM->yield();
    read_stdin_size = 0;
    stdin_keypress_size = 0;
    uint8_t buffer[1];
    buffer[0] = 0;
    int siz = Pipe::read(pipe_stdin, buffer, 1);
    *character = buffer[0];
    set_raw(false);
    return siz;
}
