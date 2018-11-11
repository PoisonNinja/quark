#include <fs/ftable.h>
#include <lib/murmur.h>
#include <lib/string.h>
#include <lib/unordered_map.h>

namespace
{
libcxx::unordered_map<libcxx::StringKey, Filesystem::Driver*,
                      libcxx::StringHash>
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