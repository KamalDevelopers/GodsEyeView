#include "multitasking.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/mouse.hpp"

BitArray<MAX_PIDS> pid_bitmap;

Task::Task(char* task_name, uint32_t eip, int privilege_level, int parent)
{
    if (strlen(task_name) > 20)
        task_name = "unknown";

    memset(arguments, 0, 500);
    memset(working_directory, 0, MAX_PATH_SIZE);
    memcpy(name, task_name, strlen(task_name));
    name[strlen(task_name)] = '\0';

    cpustate = (cpu_state*)(stack + sizeof(stack) - sizeof(cpu_state));
    cpustate->eax = 0;
    cpustate->ebx = 0;
    cpustate->ecx = 0;
    cpustate->edx = 0;
    cpustate->esi = 0;
    cpustate->edi = 0;
    cpustate->ebp = 0;
    cpustate->eip = eip;
    cpustate->cs = GDT::active->get_code_segment_selector();
    cpustate->eflags = 0x202;
    this->parent = parent;

    if (parent != -1) {
        tty = TM->task(parent)->tty;
        is_inherited_tty = true;
    } else {
        tty = new TTY;
        is_inherited_tty = false;
    }

    execute = 0;
    state = 0;
    privilege = privilege_level;
    pid = pid_bitmap.find_unset();
    pid_bitmap.bit_set(pid);
}

Task::~Task()
{
    /* FIXME: If the child runs when the parent is dead,
     *        the child tty will cause a page fault. */
    if (!is_inherited_tty)
        kfree(tty);

    if (is_executable)
        kfree((void*)loaded_executable.memory.physical_address);
    /* kfree(this); */
}

void Task::executable(executable_t exec)
{
    is_executable = true;
    loaded_executable = exec;
    cpustate->eip = exec.eip;
}

int8_t Task::notify(int signal)
{
    switch (signal) {
    case SIGILL:
        suicide(SIGILL);
        break;
    case SIGTERM:
        suicide(SIGTERM);
        break;
    case SIGQUIT:
        suicide(SIGQUIT);
        break;
    case SIGKILL:
        suicide(SIGKILL);
        break;
    case SIGHUP:
        suicide(SIGHUP);
        break;
    case SIGINT:
        suicide(SIGINT);
        break;
    }
    return 0;
}

void Task::suicide(int error)
{
    state = error;
    TM->task_has_died();
}

int Task::chdir(char* dir)
{
    char dir_path[MAX_PATH_SIZE];
    memset(dir_path, 0, MAX_PATH_SIZE);
    strcat(dir_path, working_directory);
    strcat(dir_path, dir);
    path_resolver(dir_path, true);

    if (strlen(dir_path) == 0) {
        memset(working_directory, 0, MAX_PATH_SIZE);
        return 0;
    }

    if (VFS->listdir(dir_path, 0) == -1)
        return -1;

    strcpy(working_directory, dir_path);
    return 0;
}

void Task::cwd(char* buffer)
{
    strcpy(buffer, working_directory);
}

int Task::become_tty_master()
{
    if (!is_inherited_tty)
        return 1;

    tty = new TTY;
    is_inherited_tty = true;
    return 0;
}

int Task::poll(pollfd* pollfds, uint32_t npolls)
{
    if (npolls >= sizeof(polls) / sizeof(pollfd))
        return -1;

    sleeping = -SLEEP_WAIT_POLL;
    for (uint32_t i = 0; i < npolls; i++)
        polls[i] = pollfds[i];
    num_poll = npolls;
    TM->test_poll();
    TM->yield();
    return npolls;
}

void Task::wake_from_poll()
{
    num_poll = 0;
    sleeping = 0;
}

void Task::test_poll()
{
    for (uint32_t i = 0; i < num_poll; i++) {
        if (polls[i].events & POLLIN) {
            if ((polls[i].fd == DEV_KEYBOARD_FD) && (KeyboardDriver::active->has_unread_event()))
                return wake_from_poll();
            if ((polls[i].fd == DEV_MOUSE_FD) && (MouseDriver::active->has_unread_event()))
                return wake_from_poll();
            if ((polls[i].fd == 1) && (tty->stdout_size() > 0))
                return wake_from_poll();
            if ((polls[i].fd == 0) && (!tty->is_reading_stdin()))
                return wake_from_poll();
            if (VFS->size(polls[i].fd) > 0)
                return wake_from_poll();
        }
    }
}

void Task::sleep(int sleeping_modifier)
{
    sleeping = sleeping_modifier;
}

TaskManager::TaskManager(GDT* gdt)
{
    pid_bitmap.bit_set(0);
    active = this;
    num_tasks = 0;
    current_task = -1;
}

TaskManager* TM = 0;
TaskManager::~TaskManager()
{
}

bool TaskManager::add_task(Task* task)
{
    if (strcmp(task->get_name(), "idle") == 0) {
        tasks[MAX_TASKS - 1] = task;
        return true;
    }

    if (num_tasks + 2 >= MAX_TASKS)
        return false;

    if (!is_running) {
        tasks[num_tasks] = task;
        num_tasks++;
        return true;
    }

    is_running = 0;
    tasks[num_tasks] = task;
    num_tasks++;
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
    tasks[current_task]->suicide(SIGTERM);
}

Task* TaskManager::task(int pid)
{
    for (uint32_t i = 0; i < num_tasks; i++)
        if (tasks[i]->pid == pid)
            return tasks[i];
    return 0;
}

file_table_t* TaskManager::file_table()
{
    if (testing_poll_task == -1)
        return tasks[current_task]->get_file_table();
    return tasks[testing_poll_task]->get_file_table();
}

int8_t TaskManager::send_signal(int pid, int sig)
{
    /* TODO: Implement process groups */
    if (pid == 0) {
        for (uint32_t i = 0; i < num_tasks; i++)
            if (tasks[i]->parent == tasks[current_task]->pid)
                tasks[i]->notify(sig);
        return 0;
    }

    Task* receiver = task(pid);
    if (receiver == 0)
        return -1;
    if (receiver->privilege <= tasks[current_task]->privilege)
        return -1;
    return receiver->notify(sig);
}

void TaskManager::sleep(uint32_t ticks)
{
    tasks[current_task]->sleeping = current_ticks + ticks;
}

void TaskManager::test_poll()
{
    for (uint32_t i = 0; i < num_tasks; i++) {
        if (tasks[i]->sleeping == -SLEEP_WAIT_POLL) {
            testing_poll_task = i;
            tasks[i]->test_poll();
        }

        if ((tasks[i]->sleeping == -SLEEP_WAIT_STDIN) && tasks[i]->tty->should_wake_stdin())
            return tasks[i]->wake_from_poll();
    }
    testing_poll_task = -1;
}

int TaskManager::waitpid(int pid)
{
    Task* child = task(pid);
    if (child == 0)
        return -1;

    child->wake_pid_on_exit = tasks[current_task]->get_pid();
    tasks[current_task]->sleeping = -SLEEP_WAIT_WAKE;
    return pid;
}

int TaskManager::spawn(char* file, char** args)
{
    int fd = VFS->open(file);
    if (fd < 0)
        return -1;

    int size = VFS->size(fd);
    /* FIXME: Free elfdata when task is dead */
    uint8_t* elfdata = (uint8_t*)kmalloc(size);
    VFS->read(fd, elfdata, size);
    VFS->close(fd);

    executable_t exec = Loader::load->exec(elfdata);

    if (!exec.valid)
        return -1;

    int parent_pid = (task()->parent == -1) ? task()->get_pid() : task()->parent;
    Task* child = new Task(file, 0, 0, parent_pid);
    strcpy(child->working_directory, tasks[current_task]->working_directory);
    child->executable(exec);

    if (args) {
        for (uint32_t i = 0; i < 10; i++) {
            strcat(child->arguments, args[i]);
            strcat(child->arguments, " ");
        }
    }

    child->cpustate->edx = (uint32_t)&child->arguments;
    add_task(child);
    return child->pid;
}

void TaskManager::kill_zombie_tasks()
{
    if (!check_kill)
        return;

    for (uint32_t i = 0; i < num_tasks; i++) {
        if (tasks[i]->state == ALIVE)
            continue;

        if (tasks[i]->wake_pid_on_exit)
            task(tasks[i]->wake_pid_on_exit)->wake();

        pid_bitmap.bit_clear(tasks[i]->get_pid());
        tasks[i]->~Task();
        delete_element(i, num_tasks, tasks);
        num_tasks--;
    }

    check_kill = false;
}

void TaskManager::pick_next_task()
{
    current_task++;
    scheduler_checked_tasks++;

    if (scheduler_checked_tasks > num_tasks) {
        current_task = MAX_TASKS - 1;
        return;
    }

    if (current_task >= num_tasks)
        current_task = 0;

    if (tasks[current_task]->state != ALIVE)
        pick_next_task();

    if (tasks[current_task]->sleeping < 0)
        pick_next_task();

    if (current_ticks >= tasks[current_task]->sleeping)
        tasks[current_task]->sleeping = 0;
    else
        pick_next_task();
}

cpu_state* TaskManager::schedule(cpu_state* cpustate)
{
    current_ticks++;
    if (current_task >= 0)
        tasks[current_task]->cpustate = cpustate;

    if ((num_tasks <= 0) || (is_running == 0))
        return cpustate;

    scheduler_checked_tasks = 0;
    kill_zombie_tasks();
    pick_next_task();

    return tasks[current_task]->cpustate;
}
