/*
 * Copyright (C) 2017 Jason Lu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <types.h>

namespace libcxx
{
/*
 * Taken from https://stackoverflow.com/a/34142739/8108655, with minor
 * modifications to fix some bugs and match the requirements of Quark.
 * The code is licensed under the CC-Wiki license, per
 * https://stackoverflow.blog/2009/06/25/attribution-required/
 *
 * In the event that the answer disappears or the URL changes:
 * Author: Dietmar Kühl
 * Date: Dec 07 2015 at 20:37
 * Retrieved: Nov 23 2017 at 17:58
 */
template <typename T>
class node
{
    template <typename S, libcxx::node<S> S::*>
    friend class list;
    template <typename S, libcxx::node<S> S::*>
    friend class iterator;

    T* next;
    libcxx::node<T>* prev;

public:
    node()
        : next(nullptr)
        , prev(nullptr)
    {
    }
    bool connected() const
    {
        return next != nullptr && prev != nullptr;
    }
};

template <typename T, libcxx::node<T> T::*Link>
class list
{
private:
    libcxx::node<T> content;
    size_t _size;

public:
    class iterator
    {
        template <typename S, libcxx::node<S> S::*>
        friend class list;
        libcxx::node<T>* current;

    public:
        explicit iterator(libcxx::node<T>* current)
            : current(current)
        {
        }
        T& operator*()
        {
            return *this->operator->();
        }
        T* operator->()
        {
            return this->current->next;
        }
        bool operator==(iterator const& other) const
        {
            return this->current == other.current;
        }
        bool operator!=(iterator const& other) const
        {
            return !(*this == other);
        }
        iterator& operator++()
        {
            this->current = &(this->current->next->*Link);
            return *this;
        }
        iterator operator++(int)
        {
            iterator rc(*this);
            this->operator++();
            return rc;
        }
        iterator& operator--()
        {
            this->current = this->current->prev;
            return *this;
        }
        iterator operator--(int)
        {
            iterator rc(*this);
            this->operator--();
            return rc;
        }
    };

    list()
        : _size(0)
    {
        this->content.prev = &this->content;
    }

    ~list()
    {
        // Do nothing
    }

    // It doesn't make sense to copy a list
    list(const list& other) = delete;
    list& operator=(const list& other) = delete;

    list<T, Link>::iterator begin()
    {
        return list<T, Link>::iterator(&this->content);
    }
    list<T, Link>::iterator end()
    {
        return list<T, Link>::iterator(this->content.prev);
    }

    T& front()
    {
        return *this->content.next;
    }
    T& back()
    {
        return *(this->content.prev->prev->next);
    }
    T& pop_front()
    {
        T& _front = front();
        erase(iterator_to(_front));
        return _front;
    }
    T& pop_back()
    {
        T& _back = back();
        erase(iterator_to(_back));
        return _back;
    }
    bool empty() const
    {
        return &this->content == this->content.prev;
    }
    constexpr size_t size() const
    {
        return this->size;
    }
    void push_back(T& node)
    {
        this->insert(this->end(), node);
    }
    void push_front(T& node)
    {
        this->insert(this->begin(), node);
    }
    void insert(list<T, Link>::iterator pos, T& node)
    {
        (node.*Link).next                        = pos.current->next;
        ((node.*Link).next ? (pos.current->next->*Link).prev
                           : this->content.prev) = &(node.*Link);
        (node.*Link).prev                        = pos.current;
        pos.current->next                        = &node;

        this->_size++;
    }
    list<T, Link>::iterator erase(list<T, Link>::iterator it)
    {
        libcxx::node<T>& node = (it.current->next->*Link);
        it.current->next      = (it.current->next->*Link).next;
        (it.current->next ? (it.current->next->*Link).prev
                          : this->content.prev) = it.current;
        node.prev                               = nullptr;
        node.next                               = nullptr;
        this->_size--;
        return it;
    }
    list<T, Link>::iterator iterator_to(T& value)
    {
        return list<T, Link>::iterator((value.*Link).prev);
    }
    void reset()
    {
        this->content      = libcxx::node<T>();
        this->content.prev = &this->content;
        this->_size        = 0;
    }
};
} // namespace libcxx
