#ifndef TTY_HPP
#define TTY_HPP

#include "Hardware/Drivers/qemu.hpp"
#include "pipe.hpp"
#include <LibC/poll.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/types.hpp>

#define klog QemuSerial::active->qemu_debug

class TTY {
private:
    pipe_t* pipe_stdout = 0;
    pipe_t* pipe_stdin = 0;
    int read_stdin_size = 0;
    int stdin_keypress_size = 0;

public:
    TTY();
    ~TTY();

    bool can_backspace() { return (stdin_keypress_size > 0); }
    bool is_reading_stdin() { return (read_stdin_size != 0); }
    int stdin_size() { return pipe_stdin->size; }
    int stdout_size() { return pipe_stdout->size; }
    bool should_wake_stdin();

    int write_stdin(uint8_t* buffer, uint32_t size);
    int write_stdout(uint8_t* buffer, uint32_t size);
    int read_stdin(uint8_t* buffer, uint32_t size);
    int read_stdout(uint8_t* buffer, uint32_t size);
    int task_read_stdin(uint8_t* buffer, uint32_t size);
};

#endif
