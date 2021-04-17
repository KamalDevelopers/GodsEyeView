#ifndef MULTITASKING_H
#define MULTITASKING_H

#include "GDT/gdt.hpp"
#include "LibC/stdio.hpp"
#include "LibC/types.hpp"
#include "Mem/mm.hpp"
#include "Mem/paging.hpp"
#include <stdarg.h>

#define SIG_ILL 1
#define SIG_TERM 2
#define SIG_SEGV 3

struct CPUState {
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
    uint8_t stack[4096]; // 4 KiB
    uint32_t page_directory[1024];
    CPUState* cpustate;

    int pid;
    char name[20];
    uint16_t state;
    uint8_t privelege;
    void (*notify)(int);

public:
    int8_t Notify(int signal);
    void Suicide(int error_code);
    Task(char* task_name, uint32_t entrypoint, uint8_t priv = 0);
    ~Task();
};

static uint32_t lpid = 0;
static GlobalDescriptorTable* gdt = 0;

class TaskManager {
private:
    bool AddTask(Task* task);
    int num_tasks;
    int current_task;
    int8_t is_running = 1;
    int16_t locked = -1;
    Task* tasks[256];

public:
    TaskManager(GlobalDescriptorTable* dgdt);
    ~TaskManager();

    static TaskManager* active;

    CPUState* Schedule(CPUState* cpustate);

    void Activate() { is_running = 1; }
    void Deactivate() { is_running = 0; }
    int GetPid() { return tasks[current_task]->pid; }
    char* GetName() { return tasks[current_task]->name; }

    bool AppendTasks(int count, ...);
    int8_t SendSignal(int pid, int sig);
    void KillZombieTasks();
    void Kill();

    void Lock() { locked = 0; }
    void Unlock() { locked = -1; }
};

#endif
