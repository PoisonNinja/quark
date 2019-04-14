#include <errno.h>
#include <fs/dev.h>
#include <fs/pty/pts.h>
#include <fs/stat.h>
#include <fs/tty.h>
#include <kernel.h>
#include <lib/list.h>
#include <lib/printf.h>
#include <lib/string.h>

namespace filesystem
{
namespace tty
{
pts::pts(ptm* master)
    : master(master)
{
}

libcxx::pair<int, void*> pts::open(const char* name)
{
    return libcxx::pair<int, void*>(0, nullptr);
}

ssize_t pts::write(const uint8_t* buffer, size_t count)
{
}

ssize_t pts::notify(const uint8_t* buffer, size_t count)
{
    return this->core->notify(buffer, count);
}
} // namespace tty

namespace
{
constexpr dev_t pts_major = 134;
}

ptsfs::ptsfs()
    : index(0)
{
    filesystem::register_class(filesystem::CHR, pts_major);
    this->root = new tmpfs::directory(0, 0, 0755);
}

bool ptsfs::mount(superblock* sb)
{
    sb->root = libcxx::intrusive_ptr<inode>(this->root);
    return true;
}

libcxx::pair<int, tty::tty_core*> ptsfs::register_ptm(tty::ptm* ptm)
{
    char name[128];
    tty::pts* pts      = new tty::pts(ptm);
    int real_index     = index++;
    tty::tty_core* tty = tty::register_tty(pts, pts_major, real_index);
    libcxx::sprintf(name, "pts%zu", real_index);
    this->root->mknod(name, S_IFCHR | 0644, mkdev(pts_major, real_index));
    return libcxx::make_pair(real_index, tty);
}
} // namespace filesystem
