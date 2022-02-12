#include "mutex.hpp"

bool is_enabled = false;

void Mutex::enable()
{
    is_enabled = true;
}

void Mutex::lock(Mutex::mutex_lock_t& lock)
{
    if (!is_enabled)
        return;

    if (!TM->is_active())
        return;

    while (lock.locked)
        TM->yield();

    lock.locked = 1;
}

void Mutex::unlock(Mutex::mutex_lock_t& lock)
{
    if (!is_enabled)
        return;

    if (!TM->is_active())
        return;

    lock.locked = 0;
    TM->yield();
}
