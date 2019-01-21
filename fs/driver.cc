#include <fs/driver.h>
#include <lib/murmur.h>
#include <lib/string.h>
#include <lib/unordered_map.h>

namespace
{
constexpr size_t drivers_size = 128;

libcxx::unordered_map<libcxx::string, filesystem::driver*, drivers_size>
    drivers_hash;
} // namespace

namespace filesystem
{
namespace drivers
{
bool add(const libcxx::string name, driver* driver)
{
    drivers_hash.insert(name, driver);
    return true;
}

driver* get(const libcxx::string name)
{
    driver* driver;
    if (!drivers_hash.at(name, driver)) {
        return nullptr;
    } else {
        return driver;
    }
}
} // namespace drivers
} // namespace filesystem
