#pragma once

#include <fs/vnode.h>
#include <lib/list.h>

namespace Filesystem
{
class Vnode;

struct Mount {
    Ref<Inode> target;
    Node<Mount> node;
};
}  // namespace Filesystem