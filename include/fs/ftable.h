#pragma once

#include <fs/driver.h>
#include <lib/hashmap.h>
#include <lib/murmur.h>
#include <lib/string.h>

namespace Filesystem
{
constexpr size_t ftable_size = 1024;

struct FTableHash {
    unsigned long operator()(const char* name)
    {
        return Murmur::hash(name, String::strlen(name)) % ftable_size;
    }
};

class FTable
{
public:
    FTable();
    ~FTable();
    bool add(const char* name, Driver* driver);
    Driver* get(const char* name);

private:
    Hashmap<const char*, Driver*, ftable_size, FTableHash> ftable_hash;
};
}  // namespace Filesystem