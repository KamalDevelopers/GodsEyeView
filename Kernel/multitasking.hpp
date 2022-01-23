#ifndef MULTITASKING_H
#define MULTITASKING_H

#include "Exec/loader.hpp"
#include "Filesystem/vfs.hpp"
#include "GDT/gdt.hpp"
#include "Mem/mm.hpp"
#include "tty.hpp"
#include <LibC/stdio.hpp>
#include <LibC/types.hpp>
#include <stdarg.h>

#define SIG_ILL 1
#define SIG_TERM 2
#define SIG_SEGV 3
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
    char stdin_buffer[200];

    executable_t loaded_executable;
    bool is_executable = false;
    bool is_child = false;
    uint32_t sleeping = 0;

    int pid;
    int execute;
    char name[20];
    char execfile[20];
    char arguments[500];
    uint16_t state;
    uint8_t privelege;
    void (*notify_callback)(int);

public:
    int8_t notify(int signal);
    void suicide(int error_code);
    void executable(executable_t exec);

    int get_pid() { return pid; }

    Task(char* task_name, uint32_t eip, int priv = 0);
    ~Task();
};

static uint32_t g_lpid = 0;
static GDT* g_gdt = 0;

class TaskManager {
private:
    bool add_task(Task* task);
    int num_tasks;
    int current_task;
    uint32_t current_ticks = 0;
    int8_t is_running = 1;
    bool is_reading_stdin = false;
    Task* tasks[256];

public:
    TaskManager(GDT* gdt);
    ~TaskManager();

    static TaskManager* active;

    cpu_state* schedule(cpu_state* cpustate);

    void activate() { is_running = 1; }
    void deactivate() { is_running = 0; }
    void yield() { asm volatile("int $0x20"); }
    Task* task() { return tasks[current_task]; }

    void reset_stdin();
    void append_stdin(char key);
    uint32_t read_stdin(char* buffer, uint32_t length);

    void sleep(uint32_t ticks);
    int execute(char* file);
    int spawn(char* file, char** args);
    bool append_tasks(int count, ...);
    int8_t send_signal(int pid, int sig);
    void kill_zombie_tasks();
    void kill();
};

#endif
