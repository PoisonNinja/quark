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

template <class T>
class List
{
public:
    class iterator
    {
    public:
        iterator(T ptr) : ptr(ptr)
        {
        }
        iterator operator++()
        {
            iterator i = *this;
            ptr = ptr->next;
            return i;
        }
        iterator operator++(int __attribute__((unused)) discard)
        {
            ptr = ptr->next;
            return *this;
        }
        T &operator*()
        {
            return ptr;
        }
        T *operator->()
        {
            return ptr;
        }
        bool operator==(const iterator &rhs)
        {
            return ptr == rhs.ptr;
        }
        bool operator!=(const iterator &rhs)
        {
            return ptr != rhs.ptr;
        }

    private:
        T ptr;
    };

    iterator begin()
    {
        return iterator(head);
    }

    iterator end()
    {
        return iterator(sentry);
    }

    void insert(T node)
    {
        if (!this->size) {
            this->head = node;
            node->next = sentry;
        } else {
            node->next = this->head;
            this->head = node;
        }
        this->size++;
    }

private:
    T sentry;
    T head;
    size_t size;
};
