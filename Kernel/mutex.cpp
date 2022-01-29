#include "mutex.hpp"

void Mutex::lock(Mutex::mutex_lock_t& lock)
{
    if (!TM->is_active())
        return;

    while (lock.locked) {
        TM->yield();
    }

    lock.locked = 1;
}

void Mutex::unlock(Mutex::mutex_lock_t& lock)
{
    if (!TM->is_active())
        return;

    lock.locked = 0;
    TM->yield();
}
