#include <fs/ftable.h>
#include <lib/hashmap.h>
#include <lib/murmur.h>
#include <lib/string.h>

namespace Filesystem
{
namespace FTable
{
constexpr size_t ftable_size = 1024;

struct FTableHash {
    unsigned long operator()(const char* name)
    {
        return Murmur::hash(name, String::strlen(name)) % ftable_size;
    }
};

static Hashmap<const char*, Driver*, ftable_size, FTableHash> ftable_hash;

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
}  // namespace FTable
}  // namespace Filesystem