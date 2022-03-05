#ifndef MULTITASKING_H
#define MULTITASKING_H

#include "Exec/loader.hpp"
#include "Filesystem/vfs.hpp"
#include "GDT/gdt.hpp"
#include "Mem/mm.hpp"
#include "tty.hpp"

#include <LibC++/bitarray.hpp>
#include <LibC++/vector.hpp>
#include <LibC/path.hpp>
#include <LibC/poll.hpp>
#include <LibC/stdio.hpp>
#include <LibC/types.hpp>
#include <stdarg.h>

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGIOT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15

#define ALIVE 0
#define SLEEP_WAIT_WAKE 1
#define SLEEP_WAIT_STDIN 2
#define SLEEP_WAIT_POLL 3
#define MAX_PIDS 512
#define MAX_TASKS 512
#define MAX_MEMORY_REGIONS 256
#define TM TaskManager::active

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

    TTY* tty;
    executable_t loaded_executable;
    file_table_t process_file_table;
    Vector<memory_region_t, MAX_MEMORY_REGIONS> allocated_memory;
    pollfd polls[10];

    uint32_t num_poll = 0;
    bool is_executable = false;
    bool is_inherited_tty = false;
    int sleeping = 0;
    int wake_pid_on_exit = 0;
    int process_group = 0;
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
    void sleep(int sleeping_modifier);
    void wake() { sleeping = 0; }

    int chdir(char* dir);
    void cwd(char* buffer);
    int poll(pollfd* pollfds, uint32_t npolls);

    int become_tty_master();
    void wake_from_poll();
    void test_poll();
    int setsid();

    void process_mmap(memory_region_t region);
    void process_munmap(memory_region_t region);

    file_table_t* get_file_table() { return &process_file_table; }
    int get_pid() { return pid; }
    char* get_name() { return name; }

    Task(char* task_name, uint32_t eip = 0, int privilege_level = 0, int parent = -1);
    ~Task();
};

class TaskManager {
private:
    int current_task = 0;
    int testing_poll_task = -1;
    int scheduler_checked_tasks = 0;
    uint32_t current_ticks = 0;
    bool is_running = false;
    bool check_kill = false;
    Vector<Task*, MAX_TASKS> tasks;

public:
    TaskManager(GDT* gdt);
    ~TaskManager();

    static TaskManager* active;

    cpu_state* schedule(cpu_state* cpustate);
    void pick_next_task();

    void activate() { is_running = 1; }
    void deactivate() { is_running = 0; }
    void task_has_died() { check_kill = true; }

    bool is_active() { return is_running; }
    void yield() { asm volatile("int $0x20"); }
    Task* task() { return tasks.at(current_task); }
    TTY* tty() { return tasks.at(current_task)->tty; }
    file_table_t* file_table();
    Task* task(int pid);

    void sleep(uint32_t ticks);
    int waitpid(int pid);
    void test_poll();
    int spawn(char* file, char** args);
    bool add_task(Task* task);
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
