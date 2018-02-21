#pragma once

#include <fs/inode.h>
#include <fs/vnode.h>
#include <lib/refcount.h>

namespace Filesystem
{
class Descriptor : public RefcountBase
{
public:
    ino_t ino;
    dev_t dev;
    mode_t mode;

    Descriptor(Ref<Vnode> vnode);
    Ref<Descriptor> open(const char* name, int flags, mode_t mode);
    ssize_t pread(uint8_t* buffer, size_t count, off_t offset);
    ssize_t pwrite(uint8_t* buffer, size_t count, off_t offset);
    ssize_t read(uint8_t* buffer, size_t count);
    ssize_t write(uint8_t* buffer, size_t count);

private:
    Ref<Vnode> vnode;
    off_t current_offset;
};
}
