#include "panic.hpp"
#include "Hardware/interrupts.hpp"

struct stackframe {
    struct stackframe* ebp;
    uint32_t eip;
};

void dump_stack(uint32_t base, uintptr_t ebp)
{
    struct stackframe* stk;
    stk = (struct stackframe*)ebp;

    for (unsigned int frame = 0; stk && frame < 4096; ++frame) {
        if (stk->eip)
            klog("0x%x ", stk->eip - base);
        stk = stk->ebp;
    }
}

void dump_stack()
{
    struct stackframe* stk;
    stk = (stackframe*)__builtin_frame_address(0);

    for (unsigned int frame = 0; stk && frame < 4096; ++frame) {
        if (stk->eip)
            klog("0x%x ", stk->eip);
        stk = stk->ebp;
    }
}

[[noreturn]] void panic(char* error, const char* file, uint32_t line)
{
    klog("\33\x2\x4PANIC \33\x2\xF%s : %d \33\x2\x7 * %s", file, line, error);
    IRQ::deactivate();
    klog("[stack trace]: ");
    dump_stack();

    while (1)
        ;
}

static char kernel_log_memory[KERNEL_LOG_MEMORY_SIZE];
static uint32_t kernel_log_memory_index = 0;
static uint32_t kernel_msg_index = 0;
size_t kernel_log_memory_read(char* destination, size_t size)
{
    if (size >= KERNEL_LOG_MEMORY_SIZE)
        size = KERNEL_LOG_MEMORY_SIZE - 1;
    memcpy(destination, kernel_log_memory, size);
    return size;
}

void kernel_log_write_hook(char* text)
{
    if (!kernel_log_memory_index)
        memset(kernel_log_memory, 0, sizeof(kernel_log_memory));

    uint32_t i = 0;
    for (; text[i] != 0 && (i + kernel_log_memory_index) < sizeof(kernel_log_memory); i++)
        kernel_log_memory[i + kernel_log_memory_index] = text[i];
    kernel_log_memory_index += i;

    if (QemuSerial::active->is_supported())
        QemuSerial::active->puts(text);
}

static char kernel_debug_memory[1024];
static uint32_t kernel_debug_memory_index = 0;
void kernel_debug_write_hook(char* text)
{
    if (!kernel_debug_memory_index)
        memset(kernel_debug_memory, 0, sizeof(kernel_debug_memory));
    size_t siz = strlen(text);
    if (siz + kernel_debug_memory_index > sizeof(kernel_debug_memory) || text[0] == '\n') {
        if (QemuSerial::active->is_supported() && kernel_debug_memory_index)
            QemuSerial::active->puts(kernel_debug_memory);
        memset(kernel_debug_memory, 0, kernel_debug_memory_index);
        kernel_debug_memory_index = 0;
    }
    strcat(kernel_debug_memory, text);
    kernel_debug_memory_index += siz;
}

/* Debug method (Qemu only), don't write to kern file */
void kernel_debug(const char* format, ...)
{
    va_list arg;
    puts_hook(kernel_debug_write_hook);
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
    // kernel_debug_write_hook(" \n");
    puts_hook(0);
}

/* Log method, write to kern file */
void kernel_log(const char* file, uint32_t line, const char* format, ...)
{
    kernel_msg_index++;
    const char* klog_virt_prefix = "\033[01;34m";
    const char* klog_virt_suffix = "\033[0m";

    char klog_prefix[16];
    char suffix_buffer[100];

    memset(suffix_buffer, 0, sizeof(suffix_buffer));
    snprintf(suffix_buffer, sizeof(suffix_buffer), " %s:%d\n", file, line);

    QemuSerial::active->puts((char*)klog_virt_prefix);

    memset(klog_prefix, 0, sizeof(klog_prefix));
    snprintf(klog_prefix, sizeof(klog_prefix), "[ kern :%d ] ", kernel_msg_index);
    memset(klog_prefix + strlen(klog_prefix) - 1, ' ', sizeof(klog_prefix) - strlen(klog_prefix));
    klog_prefix[sizeof(klog_prefix) - 1] = 0;

    kernel_log_write_hook(klog_prefix);

    va_list arg;
    puts_hook(kernel_log_write_hook);
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
    puts_hook(0);

    QemuSerial::active->puts("\033[100G");
    QemuSerial::active->puts((char*)klog_virt_suffix);
    QemuSerial::active->puts(suffix_buffer);

    kernel_log_memory[kernel_log_memory_index] = '\n';
    kernel_log_memory_index++;
}
