#include "multitasking.hpp"

Task::Task(char* task_name, uint32_t entrypoint, uint8_t priv)
{
    if (strlen(task_name) > 20)
        task_name = "Unknown";

    memcpy(name, task_name, strlen(task_name));
    name[strlen(task_name)] = '\0';

    cpustate = (cpu_state*)(stack + 4096 - sizeof(cpu_state));
    cpustate->eax = 0;
    cpustate->ebx = 0;
    cpustate->ecx = 0;
    cpustate->edx = 0;
    cpustate->esi = 0;
    cpustate->edi = 0;
    cpustate->ebp = 0;
    cpustate->eip = entrypoint;
    cpustate->cs = g_gdt->code_segment_selector();
    cpustate->eflags = 0x202;

    Paging::copy_page_directory(page_directory);

    state = 0;
    privelege = priv;
    pid = ++g_lpid;
}

int8_t Task::notify(int signal)
{
    switch (signal) {
    case SIG_ILL:
        suicide(SIG_ILL);
        break;
    case SIG_TERM:
        suicide(SIG_TERM);
        break;
    case SIG_SEGV:
        suicide(SIG_SEGV);
        break;
    default:
        klog("Received unknown signal");
        return -1;
    }
    return 0;
}

void Task::suicide(int error_code)
{
    state = 1;
}

Task::~Task()
{
}

TaskManager::TaskManager(GDT* gdt)
{
    g_gdt = gdt;
    active = this;
    num_tasks = 0;
    current_task = -1;
}

TaskManager* TaskManager::active = 0;
TaskManager::~TaskManager()
{
}

bool TaskManager::add_task(Task* task)
{
    if (num_tasks >= 256)
        return false;
    is_running = 0;
    tasks[num_tasks++] = task;
    is_running = 1;
    return true;
}

bool TaskManager::append_tasks(int count, ...)
{
    va_list list;
    va_start(list, count);

    for (int i = 0; i < count; i++)
        add_task(va_arg(list, Task*));
    va_end(list);
    return true;
}

void TaskManager::kill()
{
    tasks[current_task]->suicide(SIG_TERM);
}

int8_t TaskManager::send_signal(int pid, int sig)
{
    for (int i = 0; i < num_tasks; i++)
        if (tasks[i]->pid == pid)
            if (tasks[i]->privelege <= tasks[current_task]->privelege)
                return tasks[i]->notify(sig);
    return -1;
}

void TaskManager::kill_zombie_tasks()
{
    for (int i = 0; i < num_tasks; i++) {
        if (tasks[i]->state == 1) {
            klog("Zombie Process Killed");
            delete_element(i, num_tasks, tasks);
            num_tasks--;
        }
    }
}

cpu_state* TaskManager::schedule(cpu_state* cpustate)
{
    //Paging::copy_page_directory(tasks[current_task]->page_directory);
    if (locked != -1) {
        locked++;
        if (locked > 10) {
            tasks[current_task]->suicide(SIG_TERM);
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

    kill_zombie_tasks();
    if (current_task >= num_tasks)
        current_task = 0;

    Paging::switch_page_directory(tasks[current_task]->page_directory);
    return tasks[current_task]->cpustate;
}
