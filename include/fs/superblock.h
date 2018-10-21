#pragma once

#include <fs/inode.h>
#include <memory>

namespace Filesystem
{
class Vnode;

struct Superblock {
    const char* path;

    std::shared_ptr<Vnode> source; // Source file (e.g. /dev/sda)
    Ref<Inode> root;               // Filesystem specific internal inode

    dev_t rdev;
};
} // namespace Filesystem
