#pragma once

#include <fs/fs.h>
#include <fs/inode.h>

namespace Filesystem
{
class Driver
{
public:
    bool mount(Superblock* sb);
};
}  // namespace Filesystem