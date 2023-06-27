#ifndef PANIC_HPP
#define PANIC_HPP

#include <LibC/types.h>

#define PANIC(msg) panic(msg, __FILE__, __LINE__)

#define klog kernel_debug
#define KERNEL_LOG_MEMORY_SIZE 4096 // fixed size.

[[noreturn]] void panic(char* error, const char* file, uint32_t line);
void kernel_debug(const char* format, ...);
size_t kernel_log_memory_read(char* destination, size_t size);

#endif
