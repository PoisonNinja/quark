#pragma once

#include <fs/fs.h>
#include <fs/superblock.h>
#include <lib/string.h>

namespace filesystem
{
constexpr uint32_t driver_pseudo = (1 << 0);

class driver
{
public:
    virtual bool mount(superblock* sb) = 0;
    virtual uint32_t flags()           = 0;
};

namespace drivers
{
bool add(const libcxx::string name, driver* driver);
driver* get(const libcxx::string name);
} // namespace drivers
} // namespace filesystem
