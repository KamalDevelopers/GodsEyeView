#ifndef MULTITASKING_H
#define MULTITASKING_H

#include "Exec/loader.hpp"
#include "Filesystem/vfs.hpp"
#include "GDT/gdt.hpp"
#include "Mem/mm.hpp"
#include "tty.hpp"

#include <LibC/path.hpp>
#include <LibC/poll.hpp>
#include <LibC/stdio.hpp>
#include <LibC/types.hpp>
#include <stdarg.h>

#define ALIVE 0
#define SIG_ILL 1
#define SIG_TERM 2
#define SIG_SEGV 3
#define SLEEP_WAIT_WAKE 1
#define SLEEP_WAIT_STDIN 2
#define SLEEP_WAIT_POLL 2
#define MAX_PIDS 1000
#define MAX_TASKS 256
#define TM TaskManager::active
#define KB 1000

struct cpu_state {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;

    uint32_t error;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

class Task {
    friend class TaskManager;

private:
    uint8_t stack[4096];
    cpu_state* cpustate;
    char stdin_buffer[200];

    pipe_t* pipe_stdout = 0;
    pipe_t* pipe_stdin = 0;
    executable_t loaded_executable;
    file_table_t process_file_table;
    pollfd polls[10];

    uint32_t num_poll = 0;
    bool is_executable = false;
    bool is_child = false;
    int sleeping = 0;
    int wake_pid_on_exit = 0;
    int parent = -1;

    int pid;
    int execute;
    char name[20];
    char working_directory[MAX_PATH_SIZE];
    char arguments[500];
    uint16_t state;
    uint8_t privilege;
    void (*notify_callback)(int);

public:
    int8_t notify(int signal);
    void suicide(int error_code);
    void executable(executable_t exec);
    void wake() { sleeping = 0; }
    int chdir(char* dir);
    void cwd(char* buffer);
    int poll(pollfd* pollfds, uint32_t npolls);
    void wake_from_poll();
    void test_poll();

    file_table_t* get_file_table() { return &process_file_table; }
    int get_pid() { return pid; }
    char* get_name() { return name; }
    pipe_t* get_stdout() { return pipe_stdout; }
    pipe_t* get_stdin() { return pipe_stdin; }

    Task(char* task_name, uint32_t eip = 0, int privilege_level = 0, int parent = -1);
    ~Task();
};

class TaskManager {
private:
    bool add_task(Task* task);
    int num_tasks = 0;
    int current_task = 0;
    int testing_poll_task = -1;
    int scheduler_checked_tasks = 0;
    uint32_t current_ticks = 0;
    bool is_running = false;
    bool is_reading_stdin = false;
    bool check_kill = false;
    Task* tasks[MAX_TASKS];

public:
    TaskManager(GDT* gdt);
    ~TaskManager();

    static TaskManager* active;

    cpu_state* schedule(cpu_state* cpustate);
    void pick_next_task();

    bool has_task_reading_stdin() { return is_reading_stdin; }
    void activate() { is_running = 1; }
    void deactivate() { is_running = 0; }
    bool is_active() { return is_running; }
    void yield() { asm volatile("int $0x20"); }
    Task* task() { return tasks[current_task]; }
    file_table_t* get_file_table();
    Task* task(int pid);

    void reset_stdin();
    int read_stdin(char* buffer, uint32_t length);
    void write_stdin(uint8_t* buffer, uint32_t length);
    void test_poll();

    void sleep(uint32_t ticks);
    int waitpid(int pid);
    int spawn(char* file, char** args);
    bool append_tasks(int count, ...);
    int8_t send_signal(int pid, int sig);
    void kill_zombie_tasks();
    void kill();
};

inline void kernel_idle()
{
    for (;;)
        TM->yield();
}

#endif
