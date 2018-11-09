#pragma once

#include <fs/superblock.h>
#include <fs/vnode.h>
#include <lib/list.h>

namespace Filesystem
{
class Vnode;

struct Mount {
    Superblock* sb;
    libcxx::Node<Mount> node;
};
} // namespace Filesystem
