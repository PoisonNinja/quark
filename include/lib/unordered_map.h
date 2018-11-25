/*
 * Previously copied from, but now only inspired by
 * https://github.com/aozturk/HashMap
 */

#pragma once

#include <lib/functional.h>
#include <lib/list.h>
#include <lib/math.h>
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
        {
        }

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

        libcxx::node<typename unordered_map<Key, T, bucket_count>::node>
            bucket_node;

    private:
        // key-value pair
        const Key _key;
        T _value;
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
        // Destroy all buckets one by one
        for (size_t i = 0; i < this->num_buckets; ++i) {
            for (auto it = buckets[i].begin(); it != buckets[i].end();) {
                auto &obj = *it;
                delete &obj;
                it = buckets[i].erase(it);
            }
        }
    }

    bool at(const Key &key, T &value)
    {
        size_t hash_value = hash(key) % this->num_buckets;
        for (auto &node : buckets[hash_value]) {
            if (node.key() == key) {
                value = node.value();
                return true;
            }
        }

        return false;
    }

    void insert(const Key &key, const T &value)
    {
        size_t hash_value = hash(key) % this->num_buckets;

        for (auto &node : buckets[hash_value]) {
            if (node.key() == key) {
                return;
            }
        }

        typename unordered_map<Key, T, bucket_count>::node *entry =
            new typename unordered_map<Key, T, bucket_count>::node(key, value);
        buckets[hash_value].push_back(*entry);
    }

    size_t erase(const Key &key)
    {
        size_t hash_value = hash(key) % this->num_buckets;

        for (auto it = buckets[hash_value].begin();
             it != buckets[hash_value].end();) {
            if (*it.key() == key) {
                buckets[hash_value].erase(it);
                return 1;
            }
        }
        return 0;
    }

private:
    size_t num_buckets;
    libcxx::list<typename unordered_map<Key, T, bucket_count>::node,
                 &unordered_map<Key, T, bucket_count>::node::bucket_node>
        buckets[Math::log_2(bucket_count)];
    Hash hash;
};
} // namespace libcxx
