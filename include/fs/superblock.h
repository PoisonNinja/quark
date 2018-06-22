#pragma once

#include <fs/inode.h>

namespace Filesystem
{
class Vnode;

struct Superblock {
    Ref<Vnode> source;  // Source file (e.g. /dev/sda)
    Ref<Inode> root;    // Filesystem specific internal inode
};
}  // namespace Filesystem
