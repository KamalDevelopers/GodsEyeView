#ifndef EXIT_H
#define EXIT_H

#include <LibC/types.h>

int atexit(void (*func)(void));
int atexit_count();
uint32_t atexit_func(uint8_t i);

#endif
