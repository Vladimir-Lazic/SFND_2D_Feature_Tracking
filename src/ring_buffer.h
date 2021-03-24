#ifndef RING_BUFFER
#define RING_BUFFER

#include <memory>

template <class T>
class ring_buffer
{
private:
    int _buffer_size;
    int _head = 0;
    int _tail = 0;
    bool _full;

    std::unique_ptr<T[]> _buffer;

public:
    ring_buffer(int buffer_size) : _buffer(std::unique_ptr<T[]>(new T[buffer_size])), _buffer_size(buffer_size), _full(false) {}

    void insert(T item)
    {
        _buffer[_head] = item;

        if (_full)
        {
            _tail = (_tail + 1) % _buffer_size;
        }

        _head = (_head + 1) % _buffer_size;

        _full = _head == _tail;
    }
     
    T* get()
    {
        if (is_empty())
        {
            return nullptr;
        }

        return &_buffer[_tail];
    }

    void pop()
    {
        _full = false;
        _tail = (_tail + 1) % _buffer_size;
    }

    void reset()
    {
        _head = _tail;
        _full = false;
    }

    bool is_empty() const { return (_full && (_head == _tail)); }

    bool is_full() const { return _full; }

    int capacity() const { return _buffer_size; }

    int size() const
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
};

#endif // RING_BUFFER
