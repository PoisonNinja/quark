#pragma once

#include <fs/fs.h>
#include <fs/superblock.h>
#include <lib/string.h>

namespace filesystem
{
constexpr uint32_t driver_pseudo = (1 << 0);

class Driver
{
public:
    virtual bool mount(Superblock* sb) = 0;
    virtual uint32_t flags()           = 0;
};

namespace Drivers
{
bool add(const libcxx::string name, Driver* driver);
Driver* get(const libcxx::string name);
} // namespace Drivers
} // namespace filesystem
