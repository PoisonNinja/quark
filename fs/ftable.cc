#include <fs/ftable.h>

namespace Filesystem
{
FTable::FTable()
{
}

FTable::~FTable()
{
}

bool FTable::add(const char* name, Driver* driver)
{
    ftable_hash.put(name, driver);
    return true;
}

Driver* FTable::get(const char* name)
{
    Driver* driver;
    if (!ftable_hash.get(name, driver)) {
        return nullptr;
    } else {
        return driver;
    }
}
}  // namespace Filesystem