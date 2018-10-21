#include <errno.h>
#include <fs/inode.h>
#include <fs/stat.h>

namespace Filesystem
{
BaseInode::BaseInode()
{
    ino  = 0;
    rdev = 0;
    mode = 0;
}

BaseInode::~BaseInode()
{
}

int BaseInode::ioctl(unsigned long request, char* argp, void* cookie)
{
    return -EBADF;
}

int BaseInode::link(const char*, std::shared_ptr<Inode>)
{
    return -EBADF;
}

int BaseInode::mkdir(const char*, mode_t)
{
    return -EBADF;
}

int BaseInode::mknod(const char*, mode_t, dev_t)
{
    return -EBADF;
}

Pair<int, void*> BaseInode::open(const char*)
{
    // Can't use make_pair here
    return Pair<int, void*>(0, nullptr);
}

std::shared_ptr<Inode> BaseInode::lookup(const char*, int, mode_t)
{
    return std::shared_ptr<Inode>(nullptr);
}

ssize_t BaseInode::read(uint8_t*, size_t, off_t, void*)
{
    return -EBADF;
}

ssize_t BaseInode::write(uint8_t*, size_t, off_t, void*)
{
    return -EBADF;
}

int BaseInode::stat(struct stat* st)
{
    st->st_ino  = ino;
    st->st_mode = mode;
    st->st_size = size;
    st->st_uid  = uid;
    st->st_gid  = gid;
    return 0;
}

bool BaseInode::seekable()
{
    return true;
}
} // namespace Filesystem
