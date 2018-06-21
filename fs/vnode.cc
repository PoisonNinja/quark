#include <fs/vnode.h>

namespace Filesystem
{
Vnode::Vnode(Ref<Inode> inode)
{
    this->inode = inode;
    this->ino = inode->ino;
    this->dev = inode->dev;
    this->mode = inode->mode;
}

int Vnode::link(const char* name, Ref<Vnode> node)
{
    return inode->link(name, node->inode);
}

int Vnode::mkdir(const char* name, mode_t mode)
{
    return inode->mkdir(name, mode);
}

Ref<Vnode> Vnode::open(const char* name, int flags, mode_t mode)
{
    if (!mounts.empty()) {
        return mounts.front().target;
    }
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

int Vnode::stat(struct stat* st)
{
    return inode->stat(st);
}

ssize_t Vnode::write(uint8_t* buffer, size_t count)
{
    return inode->write(buffer, count);
}
}  // namespace Filesystem
