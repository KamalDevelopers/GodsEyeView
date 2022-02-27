#include "multitasking.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/mouse.hpp"

BitArray<MAX_PIDS> pid_bitmap;

Task::Task(char* task_name, uint32_t eip, int privilege_level, int parent)
{
    if (strlen(task_name) > 20)
        task_name = "unknown";

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
    cpustate->eip = eip;
    cpustate->cs = GDT::active->get_code_segment_selector();
    cpustate->eflags = 0x202;

    memset(arguments, 0, 500);
    memset(stdin_buffer, 0, 200);
    memset(working_directory, 0, MAX_PATH_SIZE);

    this->parent = parent;
    if (parent != -1) {
        pipe_stdout = TM->task(parent)->get_stdout();
        pipe_stdin = TM->task(parent)->get_stdin();
    } else {
        pipe_stdout = Pipe::create();
        pipe_stdin = Pipe::create();
    }

    execute = 0;
    state = 0;
    privilege = privilege_level;
    pid = pid_bitmap.find_unset();
    pid_bitmap.bit_set(pid);
}

Task::~Task()
{
    if (parent != -1) {
        Pipe::destroy(pipe_stdout);
        Pipe::destroy(pipe_stdin);
    }

    if (is_executable)
        kfree((void*)loaded_executable.memory.physical_address);
    kfree(this);
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
    case SIG_ILL:
        suicide(SIG_ILL);
        break;
    case SIG_TERM:
        suicide(SIG_TERM);
        break;
    case SIG_SEGV:
        suicide(SIG_SEGV);
        break;
    }
    return 0;
}

void Task::suicide(int error)
{
    state = error;
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
            if ((polls[i].fd == DEV_KEYBOARD_FD) && (KeyboardDriver::active->can_read_event()))
                return wake_from_poll();
            if ((polls[i].fd == DEV_MOUSE_FD) && (MouseDriver::active->can_read_event()))
                return wake_from_poll();
            if ((polls[i].fd == 1) && (get_stdout()->size > 0))
                return wake_from_poll();
            if (VFS->size(polls[i].fd) > 0)
                return wake_from_poll();
        }
    }
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
    check_kill = true;
    tasks[current_task]->suicide(SIG_TERM);
}

Task* TaskManager::task(int pid)
{
    for (int i = 0; i < num_tasks; i++)
        if (tasks[i]->pid == pid)
            return tasks[i];
    return 0;
}

file_table_t* TaskManager::get_file_table()
{
    if (testing_poll_task == -1)
        return tasks[current_task]->get_file_table();
    return tasks[testing_poll_task]->get_file_table();
}

int8_t TaskManager::send_signal(int pid, int sig)
{
    Task* receiver = task(pid);
    if (receiver == 0)
        return -1;
    if (receiver->privilege <= tasks[current_task]->privilege)
        return -1;
    return receiver->notify(sig);
}

void TaskManager::write_stdin(uint8_t* buffer, uint32_t length)
{
    int reading_stdin_task = -1;
    for (uint32_t i = 0; i < num_tasks; i++)
        if (tasks[i]->sleeping == -SLEEP_WAIT_STDIN)
            reading_stdin_task = i;

    if (reading_stdin_task == -1)
        return;

    tasks[reading_stdin_task]->sleeping = 0;
    Pipe::write(tasks[reading_stdin_task]->get_stdin(), buffer, length);
}

int TaskManager::read_stdin(char* buffer, uint32_t length)
{
    is_reading_stdin = true;
    tasks[current_task]->sleeping = -SLEEP_WAIT_STDIN;
    yield();
    int size = Pipe::read(tasks[current_task]->get_stdin(), (uint8_t*)buffer, length);
    is_reading_stdin = false;
    return size;
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
    uint8_t* elfdata = (uint8_t*)kmalloc(size);
    VFS->read(fd, elfdata, size);
    VFS->close(fd);

    executable_t exec = Loader::load->exec(elfdata);
    kfree(elfdata);

    if (!exec.valid)
        return -1;

    Task* child = new Task(file, 0, 0, task()->get_pid());
    strcpy(child->working_directory, tasks[current_task]->working_directory);
    child->executable(exec);
    child->is_child = true;

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
