#pragma once

#include <fs/superblock.h>
#include <fs/vnode.h>
#include <lib/list.h>

namespace filesystem
{
class Vnode;

struct Mount {
    Superblock* sb;
    libcxx::node<Mount> node;
};
} // namespace filesystem
