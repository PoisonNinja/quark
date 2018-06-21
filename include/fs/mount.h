#pragma once

#include <fs/vnode.h>
#include <lib/list.h>

namespace Filesystem
{
class Vnode;

struct Mount {
    Ref<Vnode> target;
    Node<Mount> node;
};
}  // namespace Filesystem