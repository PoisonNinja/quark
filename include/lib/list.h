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
class Node
{
    template <typename S, Node<S> S::*>
    friend class List;
    template <typename S, Node<S> S::*>
    friend class iterator;

    T* next;
    Node<T>* prev;

public:
    Node() : next(nullptr), prev(nullptr)
    {
    }
    Node(Node const&)
    {
    }
    void operator=(Node const& n)
    {
        this->next = n.next;
        this->prev = n.prev;
    }
};

template <typename T, Node<T> T::*Link>
class iterator
{
    template <typename S, Node<S> S::*>
    friend class List;
    Node<T>* current;

public:
    explicit iterator(Node<T>* current) : current(current)
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

template <typename T, Node<T> T::*Link>
class List
{
    Node<T> content;

public:
    List()
    {
        this->content.prev = &this->content;
    }
    iterator<T, Link> begin()
    {
        return iterator<T, Link>(&this->content);
    }
    iterator<T, Link> end()
    {
        return iterator<T, Link>(this->content.prev);
    }

    T& front()
    {
        return *this->content.next;
    }
    T& back()
    {
        return *(this->content.prev->prev->next);
    }
    bool empty() const
    {
        return &this->content == this->content.prev;
    }
    void push_back(T& Node)
    {
        this->insert(this->end(), Node);
    }
    void push_front(T& Node)
    {
        this->insert(this->begin(), Node);
    }
    void insert(iterator<T, Link> pos, T& Node)
    {
        (Node.*Link).next = pos.current->next;
        ((Node.*Link).next ? (pos.current->next->*Link).prev :
                             this->content.prev) = &(Node.*Link);
        (Node.*Link).prev = pos.current;
        pos.current->next = &Node;
    }
    iterator<T, Link> erase(iterator<T, Link> it)
    {
        it.current->next = (it.current->next->*Link).next;
        (it.current->next ? (it.current->next->*Link).prev :
                            this->content.prev) = it.current;
        return iterator<T, Link>(&(it.current->next->*Link));
    }
    void reset()
    {
        this->content = Node<T>();
        this->content.prev = &this->content;
    }
};
