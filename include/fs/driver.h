#pragma once

#include <fs/fs.h>
#include <fs/superblock.h>

namespace Filesystem
{
class Driver
{
public:
    virtual bool mount(Superblock* sb) = 0;
};
}  // namespace Filesystem