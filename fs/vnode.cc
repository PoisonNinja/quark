#include <fs/vnode.h>

namespace Filesystem
{
Vnode::Vnode(Ref<Inode> inode)
{
    this->inode = inode;
}

Ref<Vnode> Vnode::open(const char* name, int flags, mode_t mode)
{
    Ref<Inode> retinode = inode->open(name, flags, mode);
    if (!retinode) {
        return Ref<Vnode>(nullptr);
    }
    return Ref<Vnode>(new Vnode(retinode));
}
}
