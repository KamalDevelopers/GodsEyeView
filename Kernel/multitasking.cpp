#include "multitasking.hpp"

TaskManager* TaskManager::active = 0;
Task::Task(GlobalDescriptorTable* gdt, char* task_name, uint32_t entrypoint)
{
    if (strlen(task_name) > 20)
        task_name = "GevProcess";
    memcpy(name, task_name, strlen(task_name));
    name[strlen(task_name)] = '\0';

    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    cpustate->eax = 0;
    cpustate->ebx = 0;
    cpustate->ecx = 0;
    cpustate->edx = 0;
    cpustate->esi = 0;
    cpustate->edi = 0;
    cpustate->ebp = 0;
    cpustate->eip = entrypoint;
    cpustate->cs = gdt->CodeSegmentSelector();
    cpustate->eflags = 0x202;

    state = 0;
    pid = ++lpid;
}

void Task::Notify(int signal)
{
    switch (signal) {
    case SIG_ILL:
        Suicide(SIG_ILL);
        break;
    case SIG_TERM:
        Suicide(SIG_TERM);
        break;
    case SIG_SEGV:
        Suicide(SIG_SEGV);
        break;
    default:
        klog("Received unknown signal");
        return;
    }
}

void Task::Suicide(int error_code)
{
    state = 1;
}

Task::~Task()
{
}

TaskManager::TaskManager()
{
    active = this;
    numTasks = 0;
    currentTask = -1;
}

TaskManager::~TaskManager()
{
}

bool TaskManager::AddTask(Task* task)
{
    if (numTasks >= 256)
        return false;
    tasks[numTasks++] = task;
    return true;
}

bool TaskManager::AppendTasks(int count, ...)
{
    va_list list;
    va_start(list, count);

    for (int i = 0; i < count; i++)
        AddTask(va_arg(list, Task*));
    va_end(list);
    return true;
}

void TaskManager::Kill(int pid)
{
    if (pid >= numTasks)
        return;

    if (pid == -1)
        pid = tasks[currentTask]->pid;

    for (int i = 0; i < numTasks; i++)
        if (tasks[i]->pid == pid)
            tasks[i]->Suicide(SIG_TERM);
}

void TaskManager::SendSignal(int sig, int pid)
{
    if (pid >= numTasks)
        return;

    if (pid == -1)
        pid = tasks[currentTask]->pid;

    for (int i = 0; i < numTasks; i++)
        if (tasks[i]->pid == pid)
            tasks[i]->Notify(sig);
}

CPUState* TaskManager::Schedule(CPUState* cpustate)
{
    if (numTasks <= 0)
        return cpustate;

    if (currentTask >= 0)
        tasks[currentTask]->cpustate = cpustate;

    if (++currentTask >= numTasks)
        currentTask = 0;

    if (tasks[currentTask]->state == 1) {
        //printf("\nKilled Zombie PID: 0x%x NAME: %s", tasks[currentTask]->pid, tasks[currentTask]->name);
        klog("Zombie Killed");

        deleteElement(currentTask, numTasks, tasks);
        numTasks--;
        currentTask++;
        if (currentTask >= numTasks)
            currentTask = 0;
    }
    return tasks[currentTask]->cpustate;
}
