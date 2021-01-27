#ifndef MULTITASKING_H
#define MULTITASKING_H

#include "GDT/gdt.hpp"
#include "LibC/types.hpp"
#include "LibC/stdio.hpp"
#include <stdarg.h>

#define SIG_ILL  1
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
    CPUState* cpustate;

    int pid;
    uint16_t state;
    char name[20];
    void (*notify)(int);

public:
    void Notify(int signal);
    void Suicide() { state = 1; }
    Task(GlobalDescriptorTable* gdt, char* task_name, uint32_t entrypoint);
    ~Task();
};

static uint32_t lpid = 0;

class TaskManager {
private:
    bool AddTask(Task* task);
    int numTasks;
    int currentTask;
    Task* tasks[256];

public:
    TaskManager();
    ~TaskManager();

    static TaskManager* active;

    CPUState* Schedule(CPUState* cpustate);

    void SendSig(int sig, int pid=-1);
    void Kill(int pid=-1); 
    bool AppendTasks(int count, ...);
};

#endif
