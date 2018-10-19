#include <fs/ftable.h>
#include <lib/hashmap.h>
#include <lib/murmur.h>
#include <lib/string.h>

namespace Filesystem
{
namespace FTable
{
constexpr size_t ftable_size = 1024;

struct StringKey {
    StringKey(const char* s)
        : value(s){};
    const char* value;
    bool operator==(const struct StringKey& other)
    {
        return !String::strcmp(this->value, other.value);
    }
    bool operator!=(const struct StringKey& other)
    {
        return !(*this == other);
    }
};

struct NameHash {
    unsigned long operator()(const StringKey& k)
    {
        return Murmur::hash(k.value, String::strlen(k.value)) % ftable_size;
    }
};

static Hashmap<StringKey, Driver*, ftable_size, NameHash> ftable_hash;

bool add(const char* name, Driver* driver)
{
    ftable_hash.put(name, driver);
    return true;
}

Driver* get(const char* name)
{
    Driver* driver;
    if (!ftable_hash.get(name, driver)) {
        return nullptr;
    } else {
        return driver;
    }
}
} // namespace FTable
} // namespace Filesystem