#include <fs/vnode.h>

namespace Filesystem
{
Vnode::Vnode(Ref<Inode> inode)
{
    this->inode = inode;
}
}
