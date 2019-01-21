#pragma once

#include <fs/superblock.h>
#include <fs/vnode.h>
#include <lib/list.h>

namespace filesystem
{
class vnode;

struct mount {
    superblock* sb;
    libcxx::node<mount> node;
};
} // namespace filesystem
