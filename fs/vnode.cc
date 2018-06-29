#include <fs/vcache.h>
#include <fs/vnode.h>
#include <kernel.h>

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

int Vnode::mount(Mount* mt)
{
    this->mounts.push_front(*mt);
    return 0;
}

Ref<Vnode> Vnode::open(const char* name, int flags, mode_t mode)
{
    Ref<Inode> retinode = inode->open(name, flags, mode);
    if (!retinode) {
        return Ref<Vnode>(nullptr);
    }
    if (retinode->flags & inode_factory) {
        Log::printk(Log::INFO, "Opening factory inode...\n");
        retinode = retinode->open(name, flags, mode);
    }
    Ref<Vnode> retvnode = VCache::get(retinode->ino, retinode->dev);
    if (!retvnode) {
        Log::printk(Log::WARNING, "Failed to find %s in cache\n", name);
        retvnode = Ref<Vnode>(new Vnode(retinode));
        VCache::add(retinode->ino, retinode->dev, retvnode);
    }
    if (!retvnode->mounts.empty()) {
        Log::printk(Log::INFO, "Transitioning mountpoints\n");
        return Ref<Vnode>(new Vnode(retvnode->mounts.front().target));
    }
    return retvnode;
}

ssize_t Vnode::read(uint8_t* buffer, size_t count, off_t offset)
{
    return inode->read(buffer, count, offset);
}

ssize_t Vnode::write(uint8_t* buffer, size_t count, off_t offset)
{
    return inode->write(buffer, count, offset);
}

int Vnode::stat(struct stat* st)
{
    return inode->stat(st);
}

}  // namespace Filesystem
