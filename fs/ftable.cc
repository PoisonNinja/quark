#include <fs/ftable.h>
#include <lib/hashmap.h>
#include <lib/murmur.h>
#include <lib/string.h>

namespace
{
constexpr size_t ftable_size = 1024;

libcxx::unordered_map<libcxx::StringKey, Filesystem::Driver*, ftable_size, libcxx::StringHash<ftable_size>>
    ftable_hash;
} // namespace

namespace Filesystem
{
namespace FTable
{

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