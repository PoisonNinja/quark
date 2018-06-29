#pragma once

#include <fs/inode.h>
#include <fs/mount.h>
#include <lib/list.h>
#include <lib/refcount.h>

namespace Filesystem
{
class Vnode : public RefcountBase
{
public:
    ino_t ino;
    dev_t dev;
    mode_t mode;

    Vnode(Ref<Inode> inode);
    int link(const char* name, Ref<Vnode> node);
    int mkdir(const char* name, mode_t mode);
    int mount(Mount* mt);
    Ref<Vnode> open(const char* name, int flags, mode_t mode);
    ssize_t read(uint8_t* buffer, size_t count, off_t offset);
    ssize_t write(uint8_t* buffer, size_t count, off_t offset);
    int stat(struct stat* st);

private:
    List<Mount, &Mount::node> mounts;
    Ref<Inode> inode;
};
}  // namespace Filesystem
