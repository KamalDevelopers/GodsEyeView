#ifndef PANIC_HPP
#define PANIC_HPP

#include <LibC/types.h>

#define PANIC(msg) panic(msg, __FILE__, __LINE__)

#define klog(fmt, ...) kernel_log(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define kdbg(fmt, ...) kernel_debug(fmt, ##__VA_ARGS__)

#define KERNEL_LOG_MEMORY_SIZE 4096 // fixed size.

[[noreturn]] void panic(char* error, const char* file, uint32_t line);
void kernel_debug(const char* format, ...);
void kernel_log(const char* file, uint32_t line, const char* format, ...);
size_t kernel_log_memory_read(char* destination, size_t size);
void dump_stack(uint32_t base, uintptr_t ebp);
void dump_stack();

#endif
