#include "multitasking.hpp"

Task::Task(char* task_name, uint32_t entrypoint, uint8_t priv)
{
    if (strlen(task_name) > 20)
        task_name = "Unknown";

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

    Paging::p_copy_page_directory(page_directory);

    state = 0;
    privelege = priv;
    pid = ++lpid;
}

int8_t Task::Notify(int signal)
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
        return -1;
    }
    return 0;
}

void Task::Suicide(int error_code)
{
    state = 1;
}

Task::~Task()
{
}

TaskManager::TaskManager(GlobalDescriptorTable* dgdt)
{
    gdt = dgdt;
    active = this;
    num_tasks = 0;
    current_task = -1;
}

TaskManager* TaskManager::active = 0;
TaskManager::~TaskManager()
{
}

bool TaskManager::AddTask(Task* task)
{
    if (num_tasks >= 256)
        return false;
    is_running = 0;
    tasks[num_tasks++] = task;
    is_running = 1;
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

void TaskManager::Kill()
{
    tasks[current_task]->Suicide(SIG_TERM);
}

int8_t TaskManager::SendSignal(int pid, int sig)
{
    for (int i = 0; i < num_tasks; i++)
        if (tasks[i]->pid == pid)
            if (tasks[i]->privelege <= tasks[current_task]->privelege)
                return tasks[i]->Notify(sig);
    return -1;
}

void TaskManager::KillZombieTasks()
{
    for (int i = 0; i < num_tasks; i++) {
        if (tasks[i]->state == 1) {
            klog("Zombie Process Killed");
            deleteElement(i, num_tasks, tasks);
            num_tasks--;
        }
    }
}

CPUState* TaskManager::Schedule(CPUState* cpustate)
{
    //Paging::p_copy_page_directory(tasks[current_task]->page_directory);
    if (locked != -1) {
        locked++;
        if (locked > 10) {
            tasks[current_task]->Suicide(SIG_TERM);
            locked = -1;
        }
        return cpustate;
    }

    if ((num_tasks <= 0) || (is_running == 0))
        return cpustate;

    if (current_task >= 0)
        tasks[current_task]->cpustate = cpustate;

    if (++current_task >= num_tasks)
        current_task = 0;

    KillZombieTasks();
    if (current_task >= num_tasks)
        current_task = 0;

    Paging::p_switch_page_directory(tasks[current_task]->page_directory);
    return tasks[current_task]->cpustate;
}
