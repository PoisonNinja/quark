#pragma once

#include <fs/fs.h>
#include <fs/superblock.h>

namespace Filesystem
{
constexpr uint32_t driver_pseudo = (1 << 0);

class Driver
{
public:
    virtual bool mount(Superblock* sb) = 0;
    virtual uint32_t flags() = 0;
};
}  // namespace Filesystem