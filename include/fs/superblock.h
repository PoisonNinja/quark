#pragma once

#include <fs/inode.h>
#include <lib/memory.h>

namespace filesystem
{
class vnode;

struct superblock {
    const char* path;

    libcxx::intrusive_ptr<vnode> source; // Source file (e.g. /dev/sda)
    libcxx::intrusive_ptr<inode> root;   // filesystem specific internal inode

    dev_t rdev;
};
} // namespace filesystem
