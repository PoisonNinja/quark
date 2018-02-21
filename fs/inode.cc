#include <errno.h>
#include <fs/inode.h>

namespace Filesystem
{
BaseInode::BaseInode()
{
    ino = 0;
    dev = 0;
    mode = 0;
}

BaseInode::~BaseInode()
{
}

Ref<Inode> BaseInode::open(const char*, int, mode_t)
{
    return Ref<Inode>(nullptr);
}

ssize_t BaseInode::pread(uint8_t*, size_t, off_t)
{
    return -EBADF;
}

ssize_t BaseInode::pwrite(uint8_t*, size_t, off_t)
{
    return -EBADF;
}

ssize_t BaseInode::read(uint8_t*, size_t)
{
    return -EBADF;
}

ssize_t BaseInode::write(uint8_t*, size_t)
{
    return -EBADF;
}
}
