#pragma once

#include <types.h>

namespace libcxx
{
template <class Key>
struct hash {
    size_t operator()(Key k) const;
};

/*
 * Pointer specialization
 *
 * Uses the value of the pointer as the key
 */
template <class Key>
struct hash<Key*> {
    size_t operator()(Key* k) const
    {
        return reinterpret_cast<size_t>(k);
    }
};

template <>
struct hash<bool> {
    size_t operator()(bool k) const
    {
        return static_cast<size_t>(k);
    }
};
} // namespace libcxx