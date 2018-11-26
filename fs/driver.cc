#include <fs/ftable.h>
#include <lib/murmur.h>
#include <lib/string.h>
#include <lib/unordered_map.h>

namespace
{
constexpr size_t drivers_size = 128;

libcxx::unordered_map<libcxx::string, Filesystem::Driver*, drivers_size>
    drivers_hash;
} // namespace

namespace Filesystem
{
namespace Drivers
{

bool add(const libcxx::string name, Driver* driver)
{
    drivers_hash.insert(name, driver);
    return true;
}

Driver* get(const libcxx::string name)
{
    Driver* driver;
    if (!drivers_hash.at(name, driver)) {
        return nullptr;
    } else {
        return driver;
    }
}
} // namespace Drivers
} // namespace Filesystem
