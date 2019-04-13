#pragma once

#include <fs/dev.h>
#include <fs/inode.h>
#include <fs/mount.h>
#include <fs/poll.h>
#include <lib/list.h>
#include <lib/memory.h>
#include <lib/utility.h>

namespace filesystem
{
class vnode : public libcxx::intrusive_ref_counter
{
public:
    dev_t rdev;  // Device # if special file, ignored otherwise
    mode_t mode; // Mode of the file

    vnode(superblock* sb, libcxx::intrusive_ptr<inode> inode);
    vnode(superblock* sb, libcxx::intrusive_ptr<inode> inode, dev_t rdev);

    // Standard file operations
    int ioctl(unsigned long request, char* argp, void* cookie);
    int link(const char* name, libcxx::intrusive_ptr<vnode> node);
    libcxx::intrusive_ptr<vnode> lookup(const char* name, int flags,
                                        mode_t mode);
    int mkdir(const char* name, mode_t mode);
    int mknod(const char* name, mode_t mode, dev_t dev);
    libcxx::pair<int, void*> open(const char* name);
    int poll(poll_register_func_t& callback, void* cookie);
    ssize_t read(uint8_t* buffer, size_t count, off_t offset, void* cookie);
    ssize_t write(const uint8_t* buffer, size_t count, off_t offset,
                  void* cookie);
    int stat(struct stat* st);

    bool seekable();

    // VFS operations
    bool mounted();
    int mount(mount* mt);
    libcxx::pair<bool, struct mount*> umount();

private:
    libcxx::list<filesystem::mount, &mount::node> mounts;
    kdevice* kdev;
    superblock* sb;
    libcxx::intrusive_ptr<inode> ino;
};
} // namespace filesystem
