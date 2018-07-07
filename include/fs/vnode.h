#pragma once

#include <fs/dev.h>
#include <fs/inode.h>
#include <fs/mount.h>
#include <lib/list.h>
#include <lib/refcount.h>

namespace Filesystem
{
class Vnode : public RefcountBase
{
public:
    ino_t ino;    // Inode number
    dev_t dev;    // Device # of device this filesystem is mounted on
    dev_t rdev;   // Device # if special file, ignored otherwise
    mode_t mode;  // Mode of the file

    Vnode(Superblock* sb, Ref<Inode> inode);
    Vnode(Superblock* sb, Ref<Inode> inode, dev_t d, dev_t rd = 0);

    // Standard file operations
    int link(const char* name, Ref<Vnode> node);
    int mkdir(const char* name, mode_t mode);
    int mknod(const char* name, mode_t mode, dev_t dev);
    Ref<Vnode> open(const char* name, int flags, mode_t mode);
    ssize_t read(uint8_t* buffer, size_t count, off_t offset);
    ssize_t write(uint8_t* buffer, size_t count, off_t offset);
    int stat(struct stat* st);

    // VFS operations
    int mount(Mount* mt);

private:
    List<Mount, &Mount::node> mounts;
    KDevice* kdev;
    Superblock* sb;
    Ref<Inode> inode;
};
}  // namespace Filesystem
