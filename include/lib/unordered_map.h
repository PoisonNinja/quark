/**
 * Copyright 2017 HashMap Development Team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Borrowed from https://github.com/aozturk/HashMap
 *
 */

#pragma once

#include <lib/functional.h>
#include <lib/math.h>
#include <lib/murmur.h>
#include <lib/string.h>
#include <types.h>

namespace libcxx
{
// Hash map class template
template <class Key, class T, size_t bucket_count,
          class Hash = libcxx::hash<Key>>
class unordered_map
{
public:
    // Hash node class template
    class node
    {
    public:
        node(const Key &key, const T &value)
            : _key(key)
            , _value(value)
            , _next(nullptr)
        {
        }

        // disallow copy and assignment
        node(const node &) = delete;
        node &operator=(const node &) = delete;

        Key key() const
        {
            return _key;
        }

        T value() const
        {
            return _value;
        }

        void set_value(T value)
        {
            _value = value;
        }

        node *next() const
        {
            return _next;
        }

        void set_next(node *next)
        {
            _next = next;
        }

    private:
        // key-value pair
        const Key _key;
        T _value;
        // next bucket with the same key
        node *_next;
    };

public:
    unordered_map(const Hash &hash = Hash())
        : num_buckets(Math::log_2(bucket_count))
        , hash(hash)
    {
    }

    unordered_map(const unordered_map &other) = delete;
    const unordered_map &operator=(const unordered_map &other) = delete;

    ~unordered_map()
    {
        // destroy all buckets one by one
        for (size_t i = 0; i < this->num_buckets; ++i) {
            typename unordered_map<Key, T, bucket_count>::node *entry =
                buckets[i];

            while (entry != nullptr) {
                typename unordered_map<Key, T, bucket_count>::node *prev =
                    entry;
                entry = entry->next();
                delete prev;
            }

            buckets[i] = nullptr;
        }
    }

    bool at(const Key &key, T &value)
    {
        size_t hashValue = hash(key) % this->num_buckets;
        typename unordered_map<Key, T, bucket_count>::node *entry =
            buckets[hashValue];

        while (entry != nullptr) {
            if (entry->key() == key) {
                value = entry->value();
                return true;
            }
            entry = entry->next();
        }

        return false;
    }

    void insert(const Key &key, const T &value)
    {
        size_t hashValue = hash(key) % this->num_buckets;
        typename unordered_map<Key, T, bucket_count>::node *prev = nullptr;
        typename unordered_map<Key, T, bucket_count>::node *entry =
            buckets[hashValue];

        while (entry != nullptr && entry->key() != key) {
            prev  = entry;
            entry = entry->next();
        }

        if (entry == nullptr) {
            entry = new
                typename unordered_map<Key, T, bucket_count>::node(key, value);

            if (prev == nullptr) {
                // insert as first bucket
                buckets[hashValue] = entry;
            } else {
                prev->set_next(entry);
            }

        } else {
            // just update the value
            entry->set_value(value);
        }
    }

    size_t erase(const Key &key)
    {
        size_t hashValue = hash(key) % this->num_buckets;
        typename unordered_map<Key, T, bucket_count>::node *prev = nullptr;
        typename unordered_map<Key, T, bucket_count>::node *entry =
            buckets[hashValue];

        while (entry != nullptr && entry->key() != key) {
            prev  = entry;
            entry = entry->next();
        }

        if (entry == nullptr) {
            // key not found
            return 0;

        } else {
            if (prev == nullptr) {
                // remove first bucket of the list
                buckets[hashValue] = entry->next();

            } else {
                prev->set_next(entry->next());
            }

            delete entry;
            return 1;
        }
    }

private:
    size_t num_buckets;
    typename unordered_map<Key, T, bucket_count>::node
        *buckets[Math::log_2(bucket_count)];
    Hash hash;
};
} // namespace libcxx