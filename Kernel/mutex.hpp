#ifndef MUTEX_HPP
#define MUTEX_HPP

#include "multitasking.hpp"
#include <LibC/types.hpp>

#define MUTEX(name) static Mutex::mutex_lock_t name;

namespace Mutex {
typedef struct mutex_lock {
    uint8_t locked = 0;
} mutex_lock_t;

void lock(mutex_lock_t& lock);
void unlock(mutex_lock_t& lock);
}

#endif
