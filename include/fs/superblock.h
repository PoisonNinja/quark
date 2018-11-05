#pragma once

#include <fs/inode.h>
#include <lib/refcount.h>

namespace Filesystem
{
class Vnode;

struct Superblock {
    const char* path;

    Ref<Vnode> source;                 // Source file (e.g. /dev/sda)
    libcxx::intrusive_ptr<Inode> root; // Filesystem specific internal inode

    dev_t rdev;
};
} // namespace Filesystem
