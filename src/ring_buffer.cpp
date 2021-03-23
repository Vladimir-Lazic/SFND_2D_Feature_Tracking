#include "ring_buffer.h"

template <typename T>
void ring_buffer<T>::insert(T item)
{
    _buffer[_head] = item;

    if (_full)
    {
        _tail = (_tail + 1) % _buffer_size;
    }

    _head = (_head + 1) % _buffer_size;

    _full = _head == _tail;
}

template <typename T>
T& ring_buffer<T>::get()
{
    if (is_empty())
    {
        return T();
    }

    return _buffer[_tail];
}

template <typename T>
void ring_buffer<T>::pop()
{
    _full = false;
    _tail = (_tail + 1) % _buffer_size;
}

template <typename T>
void ring_buffer<T>::reset()
{
    _head = _tail;
    _full = false;
}

template <typename T>
bool ring_buffer<T>::is_empty() const
{
    return (_full && (_head == _tail));
}

template <typename T>
bool ring_buffer<T>::is_full() const
{
    return _full;
}

template <typename T>
int ring_buffer<T>::capacity() const
{
    return _buffer_size;
}

template <typename T>
int ring_buffer<T>::size() const
{
    int size = _buffer_size;

    if (!_full)
    {
        if (_head >= _tail)
        {
            size = _head - _tail;
        }
        else
        {
            size = _buffer_size + _head - _tail;
        }
    }

    return size;
}
