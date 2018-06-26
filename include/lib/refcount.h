/*
 * Copyright (c) 2012, 2013 Jonas 'Sortie' Termansen.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * A class that implements reference counting.
 */

#pragma once

#include <types.h>

class RefcountBase
{
public:
    RefcountBase() : count(0){};
    virtual ~RefcountBase(){};
    void increment_refcount()
    {
        count++;
    };
    void decrement_refcount()
    {
        bool need_delete = !count || !--count;
        if (need_delete) {
            delete this;
        }
    };
    size_t refcount() const
    {
        return count;
    };
    bool unique() const
    {
        return count == 1;
    };

private:
    size_t count;
};

template <class T>
class Ref
{
public:
    constexpr Ref() : obj(nullptr)
    {
    }
    explicit Ref(T* obj) : obj(obj)
    {
        if (obj)
            obj->increment_refcount();
    }
    template <class U>
    explicit Ref(U* obj) : obj(obj)
    {
        if (obj)
            obj->increment_refcount();
    }
    Ref(const Ref<T>& r) : obj(r.get())
    {
        if (obj)
            obj->increment_refcount();
    }
    template <class U>
    Ref(const Ref<U>& r) : obj(r.get())
    {
        if (obj)
            obj->increment_refcount();
    }
    ~Ref()
    {
        if (obj)
            obj->decrement_refcount();
    }

    Ref& operator=(const Ref r)
    {
        if (obj == r.get())
            return *this;
        if (obj) {
            obj->decrement_refcount();
            obj = nullptr;
        }
        if ((obj = r.get()))
            obj->increment_refcount();
        return *this;
    }

    template <class U>
    Ref operator=(const Ref<U> r)
    {
        if (obj == r.get())
            return *this;
        if (obj) {
            obj->decrement_refcount();
            obj = nullptr;
        }
        if ((obj = r.get()))
            obj->increment_refcount();
        return *this;
    }

    bool operator==(const Ref& other)
    {
        return (*this).get() == other.get();
    }

    template <class U>
    bool operator==(const Ref<U>& other)
    {
        return (*this).get() == other.get();
    }

    template <class U>
    bool operator==(const U* const& other)
    {
        return (*this).get() == other;
    }

    bool operator!=(const Ref& other)
    {
        return !((*this) == other);
    }

    template <class U>
    bool operator!=(const Ref<U>& other)
    {
        return !((*this) == other);
    }

    template <class U>
    bool operator!=(const U* const& other)
    {
        return !((*this) == other);
    }

    void reset()
    {
        if (obj)
            obj->decrement_refcount();
        obj = nullptr;
    }
    T* get() const
    {
        return obj;
    }
    T& operator*() const
    {
        return *obj;
    }
    T* operator->() const
    {
        return obj;
    }
    operator bool() const
    {
        return obj != nullptr;
    }
    size_t refcount() const
    {
        return obj ? obj->refcount : 0;
    }
    bool unique() const
    {
        return obj->unique();
    }

private:
    T* obj;
};
