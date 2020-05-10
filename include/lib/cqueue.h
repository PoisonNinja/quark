#pragma once

#include <lib/string.h>
#include <lib/utility.h>
#include <type_traits>

namespace libcxx
{
/*
 * cqueue: Circular queue for trivially copyable types (primitives, PODs).
 *         It probably doesn't make sense to be storing entire classes in
 *         circular queues.
 */
template <typename T,
          std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
class cqueue
{
private:
    T* data;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;

public:
    cqueue(size_t capacity)
        : data(new T[capacity])
        , capacity(capacity)
        , head(0)
        , tail(0)
        , count(0)
    {
    }
    ~cqueue()
    {
        delete[] data;
    }
    cqueue(const cqueue<T>& other)
        : data(new T[other.capacity])
        , capacity(other.capacity)
        , head(other.head)
        , tail(other.tail)
        , count(other.count)
    {
        // POD
        libcxx::memcpy(data, other.data, capacity * sizeof(T));
    }
    cqueue<T>& operator=(cqueue<T> other)
    {
        swap(*this, other);
        return *this;
    }
    cqueue(cqueue<T>&& other)
    {
        swap(*this, other);
    }
    cqueue<T>& operator=(cqueue<T>&& other)
    {
        swap(*this, other);
        return *this;
    }
    void swap(cqueue<T>& first, cqueue<T>& second)
    {
        libcxx::swap(first.data, second.data);
        libcxx::swap(first.capacity, second.capacity);
        libcxx::swap(first.head, second.head);
        libcxx::swap(first.tail, second.tail);
        libcxx::swap(first.count, second.count);
    }

    bool empty() const
    {
        return count == 0;
    }

    bool full() const
    {
        return count == capacity;
    }

    size_t size() const
    {
        return count;
    }

    void push(T val)
    {
        assert(!full());
        this->data[tail] = val;
        tail             = (tail + 1) % capacity;
        count++;
    }

    T pop()
    {
        assert(!empty());
        T val = this->data[head];
        head  = (head + 1) % capacity;
        count--;
        return val;
    }

    T pop_back()
    {
        assert(!empty());
        T val = this->data[tail];
        if (tail == 0) {
            tail = capacity - 1;
        } else {
            tail = tail - 1;
        }
        count--;
        return val;
    }
};
} // namespace libcxx
