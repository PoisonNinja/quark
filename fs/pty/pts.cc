#include <errno.h>
#include <fs/dev.h>
#include <fs/pty/ptm.h>
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
    return this->master->notify(buffer, count);
}

ssize_t pts::notify(const uint8_t* buffer, size_t count)
{
    return this->core->notify(buffer, count);
}

void pts::init_termios(struct ktermios& termios)
{
    termios.c_iflag  = ICRNL | IXON;
    termios.c_oflag  = OPOST | ONLCR;
    termios.c_cflag  = B38400 | CS8 | CREAD | HUPCL;
    termios.c_lflag  = ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN;
    termios.c_ispeed = termios.c_ospeed = 38400;
    libcxx::memcpy(termios.c_cc, init_cc, num_init_cc);
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

int ptsfs::register_ptm(tty::ptm* ptm)
{
    char name[128];
    tty::pts* pts = new tty::pts(ptm);
    ptm->set_pts(pts);
    int real_index = index++;
    tty::register_tty(pts, pts_major, real_index, 0);
    libcxx::sprintf(name, "%d", real_index);
    this->root->mknod(name, S_IFCHR | 0644, mkdev(pts_major, real_index));
    return real_index;
}
} // namespace filesystem
