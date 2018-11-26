/*
 * Homemade implementations of intrusive_ptr and unique_ptr
 */

#pragma once

#include <lib/utility.h>
#include <types.h>

namespace libcxx
{
class intrusive_ref_counter
{
public:
    intrusive_ref_counter()
        : count(0){};
    virtual ~intrusive_ref_counter(){};
    void intrusive_ptr_add_ref()
    {
        this->count++;
    }
    void intrusive_ptr_release()
    {
        if (!count || !--count) {
            delete this;
        }
    }
    size_t use_count()
    {
        return this->count;
    }

private:
    size_t count;
};

template <class T>
class intrusive_ptr
{
public:
    constexpr intrusive_ptr()
        : ref(nullptr){};
    constexpr intrusive_ptr(std::nullptr_t)
        : ref(nullptr){};
    explicit intrusive_ptr(T* ref)
        : ref(ref)
    {
        if (ref) {
            this->ref->intrusive_ptr_add_ref();
        }
    }
    template <class U>
    explicit intrusive_ptr(U* ref)
        : ref(ref)
    {
        if (ref) {
            this->ref->intrusive_ptr_add_ref();
        }
    }
    intrusive_ptr(const intrusive_ptr<T>& r)
        : ref(r.get())
    {
        if (this->ref) {
            this->ref->intrusive_ptr_add_ref();
        }
    }
    template <class U>
    intrusive_ptr(const intrusive_ptr<U>& r)
        : ref(r.get())
    {
        if (this->ref) {
            this->ref->intrusive_ptr_add_ref();
        }
    }
    intrusive_ptr(intrusive_ptr<T>&& r)
        : ref(r.get())
    {
        r.ref = nullptr;
    }
    template <class U>
    intrusive_ptr(intrusive_ptr<U>&& r)
        : ref(r.get())
    {
        r.ref = nullptr;
    }
    ~intrusive_ptr()
    {
        if (this->ref) {
            this->ref->intrusive_ptr_release();
        }
    }
    intrusive_ptr& operator=(const intrusive_ptr<T>& r)
    {
        if (this->ref) {
            if (this->ref == r.get()) {
                return *this;
            } else {
                this->ref->intrusive_ptr_release();
            }
        }
        this->ref = r.get();
        if (this->ref) {
            this->ref->intrusive_ptr_add_ref();
        }
        return *this;
    }
    template <class U>
    intrusive_ptr& operator=(const intrusive_ptr<U>& r)
    {
        if (this->ref) {
            if (this->ref == r.get()) {
                return *this;
            } else {
                this->ref->intrusive_ptr_release();
            }
        }
        this->ref = r.get();
        if (this->ref) {
            this->ref->intrusive_ptr_add_ref();
        }
        return *this;
    }
    void reset()
    {
        if (this->ref) {
            this->ref->intrusive_ptr_release();
        }
        this->ref = nullptr;
    }
    void reset(T* ptr)
    {
        intrusive_ptr<T>(ptr).swap(*this);
    }
    template <class U>
    void reset(U* ptr)
    {
        intrusive_ptr<U>(ptr).swap(*this);
    }
    void swap(intrusive_ptr<T>& r)
    {
        libcxx::swap(this->ref, r.ref);
    }
    T* get() const
    {
        return ref;
    }
    T& operator*() const
    {
        return *ref;
    }
    T* operator->() const
    {
        return ref;
    }
    long use_count() const
    {
        return (this->ref) ? this->ref->use_count() : 0;
    }
    bool unique() const
    {
        return this->use_count() == 1;
    }
    operator bool() const
    {
        return ref != nullptr;
    }

    bool operator==(const intrusive_ptr<T>& rhs)
    {
        return this->get() == rhs.get();
    }
    template <class U>
    bool operator==(const intrusive_ptr<U>& rhs)
    {
        return this->get() == rhs.get();
    }
    bool operator!=(const intrusive_ptr<T>& rhs)
    {
        return !(*this == rhs);
    }
    template <class U>
    bool operator!=(const intrusive_ptr<U>& rhs)
    {
        return !(*this == rhs);
    }

private:
    T* ref;

    template <class U>
    friend class intrusive_ptr;
};

template <class _Tp>
inline constexpr _Tp* addressof(_Tp& __x)
{
    return __builtin_addressof(__x);
}
} // namespace libcxx
