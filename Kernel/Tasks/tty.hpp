#ifndef TTY_HPP
#define TTY_HPP

#include "../Hardware/Drivers/virtual.hpp"
#include "../panic.hpp"
#include "../pipe.hpp"
#include <LibC/poll.h>
#include <LibC/stdlib.h>
#include <LibC/types.h>

class TTY {
private:
    pipe_t* pipe_stdout = 0;
    pipe_t* pipe_stdin = 0;
    int read_stdin_size = 0;
    int stdin_keypress_size = 0;
    bool stdin_raw_mode = false;

public:
    TTY();
    ~TTY();

    bool can_backspace() { return (stdin_keypress_size > 0); }
    bool is_reading_stdin() { return (read_stdin_size != 0); }
    int stdin_size() { return pipe_stdin->size; }
    int stdout_size() { return pipe_stdout->size; }
    bool should_wake_stdin();

    void set_raw(bool mode);
    int write_stdin(uint8_t* buffer, uint32_t size);
    int write_stdout(uint8_t* buffer, uint32_t size);
    int read_stdin(uint8_t* buffer, uint32_t size);
    int read_stdout(uint8_t* buffer, uint32_t size);
    int task_read_stdin(uint8_t* buffer, uint32_t size);
    int task_getchar(int* character);
};

#endif
