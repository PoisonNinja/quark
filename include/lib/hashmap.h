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

#include <types.h>

template <typename K, size_t tableSize>
struct KeyHash {
    unsigned long operator()(const K &key) const
    {
        return reinterpret_cast<unsigned long>(key) % tableSize;
    }
};

// Hash node class template
template <typename K, typename V>
class HashNode
{
public:
    HashNode(const K &key, const V &value)
        : _key(key)
        , _value(value)
        , _next(nullptr)
    {
    }

    K key() const
    {
        return _key;
    }

    V value() const
    {
        return _value;
    }

    void set_value(V value)
    {
        _value = value;
    }

    HashNode *next() const
    {
        return _next;
    }

    void set_next(HashNode *next)
    {
        _next = next;
    }

private:
    // key-value pair
    const K _key;
    V _value;
    // next bucket with the same key
    HashNode *_next;
    // disallow copy and assignment
    HashNode(const HashNode &);
    HashNode &operator=(const HashNode &);
};

// Hash map class template
template <typename K, typename V, size_t tableSize,
          typename F = KeyHash<K, tableSize>>
class Hashmap
{
public:
    Hashmap()
        : table()
        , hash()
    {
    }

    ~Hashmap()
    {
        // destroy all buckets one by one
        for (size_t i = 0; i < tableSize; ++i) {
            HashNode<K, V> *entry = table[i];

            while (entry != nullptr) {
                HashNode<K, V> *prev = entry;
                entry                = entry->next();
                delete prev;
            }

            table[i] = nullptr;
        }
    }

    bool get(const K &key, V &value)
    {
        unsigned long hashValue = hash(key);
        HashNode<K, V> *entry   = table[hashValue];

        while (entry != nullptr) {
            if (entry->key() == key) {
                value = entry->value();
                return true;
            }

            entry = entry->next();
        }

        return false;
    }

    void put(const K &key, const V &value)
    {
        unsigned long hashValue = hash(key);
        HashNode<K, V> *prev    = nullptr;
        HashNode<K, V> *entry   = table[hashValue];

        while (entry != nullptr && entry->key() != key) {
            prev  = entry;
            entry = entry->next();
        }

        if (entry == nullptr) {
            entry = new HashNode<K, V>(key, value);

            if (prev == nullptr) {
                // insert as first bucket
                table[hashValue] = entry;

            } else {
                prev->set_next(entry);
            }

        } else {
            // just update the value
            entry->set_value(value);
        }
    }

    void remove(const K &key)
    {
        unsigned long hashValue = hash(key);
        HashNode<K, V> *prev    = nullptr;
        HashNode<K, V> *entry   = table[hashValue];

        while (entry != nullptr && entry->key() != key) {
            prev  = entry;
            entry = entry->next();
        }

        if (entry == nullptr) {
            // key not found
            return;

        } else {
            if (prev == nullptr) {
                // remove first bucket of the list
                table[hashValue] = entry->next();

            } else {
                prev->set_next(entry->next());
            }

            delete entry;
        }
    }

private:
    Hashmap(const Hashmap &other);
    const Hashmap &operator=(const Hashmap &other);
    // hash table
    HashNode<K, V> *table[tableSize];
    F hash;
};
