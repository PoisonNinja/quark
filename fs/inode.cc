#include <errno.h>
#include <fs/inode.h>
#include <fs/stat.h>

namespace filesystem
{
inode::inode()
{
    ino  = 0;
    rdev = 0;
    mode = 0;
}

inode::~inode()
{
}

int inode::ioctl(unsigned long request, char* argp, void* cookie)
{
    return -EBADF;
}

int inode::link(const char*, libcxx::intrusive_ptr<inode>)
{
    return -EBADF;
}

int inode::mkdir(const char*, mode_t)
{
    return -EBADF;
}

int inode::mknod(const char*, mode_t, dev_t)
{
    return -EBADF;
}

libcxx::pair<int, void*> inode::open(const char*)
{
    // Can't use make_pair here
    return libcxx::pair<int, void*>(0, nullptr);
}

int inode::poll(poll_register_func_t& callback, void* cookie)
{
    // Regular files are always ready to read, unless overridden
    return POLLIN;
}

libcxx::intrusive_ptr<inode> inode::lookup(const char*, int, mode_t)
{
    return libcxx::intrusive_ptr<inode>(nullptr);
}

ssize_t inode::read(uint8_t*, size_t, off_t, void*)
{
    return -EBADF;
}

ssize_t inode::write(const uint8_t*, size_t, off_t, void*)
{
    return -EBADF;
}

int inode::stat(struct stat* st)
{
    st->st_ino  = ino;
    st->st_mode = mode;
    st->st_size = size;
    st->st_uid  = uid;
    st->st_gid  = gid;
    return 0;
}

bool inode::seekable()
{
    return true;
}
} // namespace filesystem
