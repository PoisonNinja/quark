#pragma once

#include <fs/inode.h>
#include <lib/memory.h>

namespace filesystem
{
class Vnode;

struct Superblock {
    const char* path;

    libcxx::intrusive_ptr<Vnode> source; // Source file (e.g. /dev/sda)
    libcxx::intrusive_ptr<Inode> root;   // filesystem specific internal inode

    dev_t rdev;
};
} // namespace filesystem
