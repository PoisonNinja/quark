#pragma once

#include <fs/inode.h>
#include <fs/vnode.h>
#include <lib/refcount.h>

namespace Filesystem
{
class Descriptor : public RefcountBase
{
public:
    Descriptor(Ref<Vnode> vnode);
    int link(const char* name, Ref<Descriptor> node);
    off_t lseek(off_t offset, int whence);
    int mkdir(const char* name, mode_t mode);
    int mknod(const char* name, mode_t mode, dev_t dev);
    int mount(const char* source, const char* target, const char* type,
              unsigned long flags);
    Ref<Descriptor> open(const char* name, int flags, mode_t mode);
    ssize_t pread(uint8_t* buffer, size_t count, off_t offset);
    ssize_t pwrite(uint8_t* buffer, size_t count, off_t offset);
    ssize_t read(uint8_t* buffer, size_t count);
    bool seekable();
    int stat(struct stat* st);
    ssize_t write(uint8_t* buffer, size_t count);

private:
    Ref<Vnode> vnode;
    off_t current_offset;
};
} // namespace Filesystem
