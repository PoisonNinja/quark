#include <errno.h>
#include <fs/inode.h>
#include <fs/stat.h>

namespace Filesystem
{
Inode::Inode()
{
    ino  = 0;
    rdev = 0;
    mode = 0;
}

Inode::~Inode()
{
}

int Inode::ioctl(unsigned long request, char* argp, void* cookie)
{
    return -EBADF;
}

int Inode::link(const char*, libcxx::intrusive_ptr<Inode>)
{
    return -EBADF;
}

int Inode::mkdir(const char*, mode_t)
{
    return -EBADF;
}

int Inode::mknod(const char*, mode_t, dev_t)
{
    return -EBADF;
}

libcxx::pair<int, void*> Inode::open(const char*)
{
    // Can't use make_pair here
    return libcxx::pair<int, void*>(0, nullptr);
}

libcxx::intrusive_ptr<Inode> Inode::lookup(const char*, int, mode_t)
{
    return libcxx::intrusive_ptr<Inode>(nullptr);
}

ssize_t Inode::read(uint8_t*, size_t, off_t, void*)
{
    return -EBADF;
}

ssize_t Inode::write(const uint8_t*, size_t, off_t, void*)
{
    return -EBADF;
}

int Inode::stat(struct stat* st)
{
    st->st_ino  = ino;
    st->st_mode = mode;
    st->st_size = size;
    st->st_uid  = uid;
    st->st_gid  = gid;
    return 0;
}

bool Inode::seekable()
{
    return true;
}
} // namespace Filesystem
