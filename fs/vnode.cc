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

ssize_t Vnode::pread(uint8_t* buffer, size_t count, off_t offset)
{
    return inode->pread(buffer, count, offset);
}

ssize_t Vnode::pwrite(uint8_t* buffer, size_t count, off_t offset)
{
    return inode->pwrite(buffer, count, offset);
}

ssize_t Vnode::read(uint8_t* buffer, size_t count)
{
    return inode->read(buffer, count);
}

ssize_t Vnode::write(uint8_t* buffer, size_t count)
{
    return inode->write(buffer, count);
}
}
