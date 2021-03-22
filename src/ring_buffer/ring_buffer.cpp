#include "ring_buffer.h"

template <typename T>
void ring_buffer<T>::insert(T item)
{
    return;
}

template <typename T>
T ring_buffer<T>::get()
{
    return T();
}

template <typename T>
void ring_buffer<T>::reset()
{
    return;
}

template <typename T>
bool ring_buffer<T>::is_empty() const
{
    return false;
}

template <typename T>
bool ring_buffer<T>::is_full() const
{
    return false;
}

template <typename T>
int ring_buffer<T>::capacity() const
{
    return 0;
}

template <typename T>
int ring_buffer<T>::size() const
{
    return 0;
}
