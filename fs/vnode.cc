#include <fs/vcache.h>
#include <fs/vnode.h>
#include <kernel.h>

namespace Filesystem
{
Vnode::Vnode(Superblock* sb, Ref<Inode> inode) : Vnode(sb, inode, inode->dev, 0)
{
}

Vnode::Vnode(Superblock* sb, Ref<Inode> inode, dev_t d, dev_t rd)
{
    this->sb = sb;
    this->inode = inode;
    this->ino = inode->ino;
    this->dev = d;
    this->rdev = rd;
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

int Vnode::mknod(const char* name, mode_t mode, dev_t dev)
{
    return inode->mknod(name, mode, dev);
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
    Ref<Vnode> retvnode = VCache::get(retinode->ino, this->sb->rdev);
    if (!retvnode) {
        Log::printk(Log::WARNING, "Failed to find %s in cache\n", name);
        retvnode = Ref<Vnode>(new Vnode(this->sb, retinode, this->sb->rdev, 0));
        VCache::add(retvnode->ino, retvnode->dev, retvnode);
    } else {
        // New Vnodes cannot have a mountpoint so we can bypass this check if
        // the vnode was just created
        if (!retvnode->mounts.empty()) {
            Superblock* newsb = retvnode->mounts.front().sb;
            Log::printk(Log::INFO,
                        "Transitioning mountpoints, superblock at %p\n", newsb);
            return Ref<Vnode>(new Vnode(newsb, newsb->root, newsb->rdev, 0));
        }
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
