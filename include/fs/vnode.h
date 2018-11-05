#pragma once

#include <fs/dev.h>
#include <fs/inode.h>
#include <fs/mount.h>
#include <lib/list.h>
#include <lib/memory.h>
#include <lib/utility.h>

namespace Filesystem
{
class Vnode : public libcxx::intrusive_ref_counter
{
public:
    dev_t rdev;  // Device # if special file, ignored otherwise
    mode_t mode; // Mode of the file

    Vnode(Superblock* sb, libcxx::intrusive_ptr<Inode> inode);
    Vnode(Superblock* sb, libcxx::intrusive_ptr<Inode> inode, dev_t rdev);

    // Standard file operations
    int ioctl(unsigned long request, char* argp, void* cookie);
    int link(const char* name, libcxx::intrusive_ptr<Vnode> node);
    libcxx::intrusive_ptr<Vnode> lookup(const char* name, int flags,
                                        mode_t mode);
    int mkdir(const char* name, mode_t mode);
    int mknod(const char* name, mode_t mode, dev_t dev);
    virtual libcxx::pair<int, void*> open(const char* name);
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
    libcxx::intrusive_ptr<Inode> inode;
};
} // namespace Filesystem
