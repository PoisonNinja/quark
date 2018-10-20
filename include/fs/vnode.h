#pragma once

#include <fs/dev.h>
#include <fs/inode.h>
#include <fs/mount.h>
#include <lib/list.h>
#include <lib/pair.h>
#include <lib/refcount.h>

namespace Filesystem
{
class Vnode : public RefcountBase
{
public:
    dev_t rdev;  // Device # if special file, ignored otherwise
    mode_t mode; // Mode of the file

    Vnode(Superblock* sb, Ref<Inode> inode);
    Vnode(Superblock* sb, Ref<Inode> inode, dev_t rdev);

    // Standard file operations
    int ioctl(unsigned long request, char* argp, void* cookie);
    int link(const char* name, Ref<Vnode> node);
    Ref<Vnode> lookup(const char* name, int flags, mode_t mode);
    int mkdir(const char* name, mode_t mode);
    int mknod(const char* name, mode_t mode, dev_t dev);
    virtual Pair<int, void*> open(const char* name);
    ssize_t read(uint8_t* buffer, size_t count, off_t offset, void* cookie);
    ssize_t write(uint8_t* buffer, size_t count, off_t offset, void* cookie);
    int stat(struct stat* st);

    bool seekable();

    // VFS operations
    int mount(Mount* mt);

private:
    List<Mount, &Mount::node> mounts;
    KDevice* kdev;
    Superblock* sb;
    Ref<Inode> inode;
};
} // namespace Filesystem
