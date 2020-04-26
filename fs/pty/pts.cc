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
namespace terminal
{
pts::pts(tty* master)
    : master(master)
{
}

int pts::open(const char* name)
{
    return 0;
}

ssize_t pts::write(const uint8_t* buffer, size_t count)
{
    return this->master->notify(buffer, count);
}

void pts::init_termios(struct termios& termios)
{
    termios.c_iflag  = ICRNL | IXON;
    termios.c_oflag  = OPOST | ONLCR;
    termios.c_cflag  = B38400 | CS8 | CREAD | HUPCL;
    termios.c_lflag  = ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN;
    termios.c_ispeed = termios.c_ospeed = 38400;
    libcxx::memcpy(termios.c_cc, init_cc, num_init_cc);
}
} // namespace terminal

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

int ptsfs::register_ptm(terminal::tty* ptm)
{
    char name[128];
    terminal::pts* pts = new terminal::pts(ptm);
    int real_index     = index++;
    register_tty(pts, pts_major, real_index, 0);
    libcxx::sprintf(name, "%d", real_index);
    this->root->mknod(name, S_IFCHR | 0644, mkdev(pts_major, real_index));
    return mkdev(pts_major, real_index);
}
} // namespace filesystem
