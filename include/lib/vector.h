#pragma once

#include <lib/algorithm.h>
#include <lib/utility.h>
#include <types.h>

namespace libcxx
{
template <class T>
class vector
{
public:
    vector()
        : _data(nullptr)
        , _capacity(0)
        , _size(0){};
    explicit vector(size_t count)
        : vector()
    {
        this->resize(count);
    };

    explicit vector(size_t count, const T& value)
        : vector()
    {
        this->resize(count, value);
    }

    vector(const vector<T>& other)
        : vector()
    {
        this->resize(other._capacity);
        for (size_t i = 0; i < other._size; i++) {
            this->_data[i] = other._data[i];
        }
        this->_size = other._size;
    }

    vector(vector<T>&& other)
    {
        swap(other);
    }

    vector<T>& operator=(const vector<T>& other)
    {
        vector<T> tmp(other);
        swap(tmp);
        return *this;
    }

    vector<T>& operator=(vector<T>&& other)
    {
        swap(other);
        return *this;
    }

    ~vector()
    {
        delete[] _data;
    }

    size_t capacity() const
    {
        return _capacity;
    }
    size_t size() const
    {
        return _size;
    }
    void reserve(size_t cap)
    {
        this->resize(cap);
    }

    void push_back(const T& value)
    {
        if (_size == _capacity) {
            resize((_capacity) ? _capacity * 3 / 2 : 4);
        }
        this->_data[_size++] = value;
    }

    T& front()
    {
        // Calling this on uninitialized is UB
        return _data[0];
    }

    T& back()
    {
        return _data[_size - 1];
    }

    T* data()
    {
        return _data;
    }

    bool at(size_t pos, T& val)
    {
        if (pos > _size) {
            return false;
        }
        val = _data[pos];
        return true;
    }

    T& operator[](size_t pos)
    {
        return _data[pos];
    }

    bool operator==(const libcxx::vector<T> other)
    {
        if (this->_size != other._size) {
            return false;
        }
        for (size_t i = 0; i < _size; i++) {
            if (_data[i] != other._data[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const libcxx::vector<T> other)
    {
        return !(*this == other);
    }

    T* begin()
    {
        return this->_data;
    }

    /*
     * Dereferencing this will probably cause a page fault if _size == _capacity
     * because it points to beyond end of _data, but thankfully dereferencing
     * the end() iterator is UB.
     */
    T* end()
    {
        return this->_data + _size;
    }

    const T* cbegin()
    {
        return this->_data;
    }

    const T* cend()
    {
        return this->_data + _size;
    }

    void swap(libcxx::vector<T>& other)
    {
        libcxx::swap(_data, other._data);
        libcxx::swap(_size, other._size);
        libcxx::swap(_capacity, other._capacity);
    }

private:
    void resize(size_t new_size, T val = T())
    {
        T* tmp = new T[new_size];
        if (this->_data) {
            for (size_t i = 0; i < libcxx::min(_size, new_size); i++) {
                tmp[i] = _data[i];
            }
            if (new_size > _size) {
                for (size_t i = _size; i < new_size; i++) {
                    tmp[i] = val;
                }
            }
        }
        libcxx::swap(tmp, _data);
        libcxx::swap(_capacity, new_size);
        delete[] tmp;
    }

    T* _data;
    size_t _capacity;
    size_t _size;
};
} // namespace libcxx
