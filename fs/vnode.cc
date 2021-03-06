#include <fs/descriptor.h>
#include <fs/fs.h>
#include <fs/stat.h>
#include <fs/vcache.h>
#include <fs/vnode.h>
#include <kernel.h>

namespace filesystem
{
vnode::vnode(superblock* sb, libcxx::intrusive_ptr<inode> inode)
    : vnode(sb, inode, inode->rdev)
{
}

vnode::vnode(superblock* sb, libcxx::intrusive_ptr<inode> inode, dev_t rdev)
{
    this->sb   = sb;
    this->ino  = inode;
    this->rdev = rdev;
    this->mode = ino->mode;
    this->kdev = nullptr;
}

int vnode::ioctl(unsigned long request, char* argp)
{
    if (this->kdev) {
        return this->kdev->ioctl(request, argp);
    }
    return this->ino->ioctl(request, argp);
}

int vnode::link(const char* name, libcxx::intrusive_ptr<vnode> node)
{
    return ino->link(name, node->ino);
}

int vnode::mkdir(const char* name, mode_t mode)
{
    return ino->mkdir(name, mode);
}

int vnode::mknod(const char* name, mode_t mode, dev_t dev)
{
    // TODO: Check if file exists
    return this->ino->mknod(name, mode, dev);
}

bool vnode::mounted()
{
    return !(this->mounts.empty());
}

int vnode::mount(filesystem::mount* mt)
{
    this->mounts.push_front(*mt);
    return 0;
}

libcxx::optional<struct mount*> vnode::umount()
{
    if (this->mounts.empty()) {
        return libcxx::nullopt;
    } else {
        struct mount* mt = &(this->mounts.front());
        this->mounts.erase(this->mounts.begin());
        return {mt};
    }
}

libcxx::intrusive_ptr<vnode> vnode::lookup(const char* name, int flags,
                                           mode_t mode)
{
    libcxx::intrusive_ptr<inode> retinode = ino->lookup(name, flags, mode);
    if (!retinode) {
        return libcxx::intrusive_ptr<vnode>(nullptr);
    }
    libcxx::intrusive_ptr<vnode> retvnode =
        vcache::get(retinode->ino, this->sb->rdev);
    if (!retvnode) {
        struct kdevice* kdev = get_kdevice(retinode->mode, retinode->rdev);
        if (S_ISBLK(retinode->mode) || S_ISCHR(retinode->mode)) {
            log::printk(log::log_level::DEBUG,
                        "kdev: Looking for mode %X and dev %p\n",
                        retinode->mode, retinode->rdev);
            if (!kdev) {
                // TODO: Return -ENXIO
                return libcxx::intrusive_ptr<vnode>(nullptr);
            }
        }
        log::printk(log::log_level::DEBUG, "Failed to find %s in cache\n",
                    name);
        retvnode =
            libcxx::intrusive_ptr<vnode>(new vnode(this->sb, retinode, 0));
        vcache::add(retinode->ino, this->sb->rdev, retvnode);
        if (S_ISBLK(retinode->mode) || S_ISCHR(retinode->mode)) {
            retvnode->kdev = kdev->factory();
        }
    } else {
        // New Vnodes cannot have a mountpoint so we can bypass this check
        // if the vnode was just created
        if (!(flags & descriptor_flags::F_NOMOUNT)) {
            if (!retvnode->mounts.empty()) {
                superblock* newsb = retvnode->mounts.front().sb;
                log::printk(log::log_level::DEBUG,
                            "Transitioning mountpoints, superblock at %p\n",
                            newsb);
                return libcxx::intrusive_ptr<vnode>(
                    new vnode(newsb, newsb->root, 0));
            }
        }
    }
    return retvnode;
}

int vnode::open(const char* name)
{
    if (this->kdev) {
        return this->kdev->open(name);
    }
    return this->ino->open(name);
}

int vnode::poll(poll_register_func_t& callback)
{
    if (this->kdev) {
        return this->kdev->poll(callback);
    }
    return this->ino->poll(callback);
}

ssize_t vnode::read(uint8_t* buffer, size_t count, off_t offset)
{
    if (this->kdev) {
        return this->kdev->read(buffer, count, offset);
    }
    return ino->read(buffer, count, offset);
}

ssize_t vnode::write(const uint8_t* buffer, size_t count, off_t offset)
{
    if (this->kdev) {
        return this->kdev->write(buffer, count, offset);
    }
    return ino->write(buffer, count, offset);
}

int vnode::stat(struct stat* st)
{
    st->st_dev = this->sb->rdev;
    return ino->stat(st);
}

bool vnode::seekable()
{
    if (this->kdev) {
        return this->kdev->seekable();
    }
    return this->ino->seekable();
}
} // namespace filesystem
