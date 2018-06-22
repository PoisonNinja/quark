#pragma once

#include <fs/inode.h>
#include <lib/refcount.h>

namespace Filesystem
{
class Vnode;

struct Superblock : public RefcountBase {
    Ref<Vnode> target;
    Ref<Inode> root;
};
}  // namespace Filesystem
