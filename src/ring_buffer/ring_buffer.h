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

    void insert(T item);
    T get();
    void reset();
    bool is_empty() const;
    bool is_full() const;
    int capacity() const;
    int free_space() const;
};

#endif // RING_BUFFER
