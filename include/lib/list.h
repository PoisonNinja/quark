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
class Node
{
public:
    Node *next, *prev;
    T *data;
};

template <class T>
class List
{
public:
    void insert(Node<T> *node);

private:
    Node<T> *head;
    size_t size;
};

/*
 * A simplified version of the Linux linked list implementation. Everything
 * is self written, except for the containerof macro
 */
#define containerof(ptr, type, member)                     \
    ({                                                     \
        const typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member)); \
    })

struct list_element {
    struct list_element *next, *prev;
};

#define LIST_COMPILE_INIT(list)       \
    {                                 \
        .next = &list, .prev = &list, \
    }

static inline void list_runtime_init(struct list_element *list)
{
    list->next = list;
    list->prev = list;
}

static inline void list_add(struct list_element *list,
                            struct list_element *element)
{
    element->next = list->next;
    element->prev = list;
    list->next->prev = element;
    list->next = element;
}

static inline void list_add_tail(struct list_element *list,
                                 struct list_element *element)
{
    element->next = list;
    element->prev = list->prev;
    list->prev->next = element;
    list->prev = element;
}

static inline void list_delete(struct list_element *element)
{
    element->next->prev = element->prev;
    element->prev->next = element->next;
    element->next = NULL;
    element->prev = NULL;
}

static inline int list_empty(struct list_element *list)
{
    return list->next == list;
}

#define list_first_entry(list, type, member) \
    containerof((list)->next, type, member)

#define list_next_entry(pos, member) \
    containerof((pos)->member.next, typeof(*(pos)), member)

#define list_for_each(list, member, pos)                          \
    for ((pos) = containerof((list)->next, typeof(*pos), member); \
         &(pos)->member != (list);                                \
         pos = containerof((pos)->member.next, typeof(*pos), member))
