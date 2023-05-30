#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <LibC/types.h>

template<typename T, size_t storage_capacity>
class Vector {
private:
    T storage[storage_capacity];
    uint32_t storage_size = 0;

public:
    T at(uint32_t index)
    {
        if (index >= storage_size)
            return {};
        return storage[index];
    }

    int remove_at(uint32_t index)
    {
        if (index >= storage_size)
            return -1;
        for (int i = index; i < storage_size - 1; i++)
            storage[i] = storage[i + 1];
        storage_size--;
        return 0;
    }

    int append(T element)
    {
        if (is_full())
            return -1;

        storage[storage_size] = element;
        storage_size++;
        return 0;
    }

    T& operator[](size_t index) { return storage[index]; }
    T& last() { return storage[storage_size - 1]; }
    bool is_full() { return (storage_size + 1 >= storage_capacity); }
    int size() { return storage_size; }
    int max_size() { return storage_capacity; }
    T* elements() { return storage; }
};

#endif
