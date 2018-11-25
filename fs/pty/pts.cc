#include <errno.h>
#include <fs/dev.h>
#include <fs/pty/pts.h>
#include <fs/stat.h>
#include <fs/tty.h>
#include <kernel.h>
#include <lib/list.h>
#include <lib/printf.h>
#include <lib/string.h>

// HACK
#include <proc/sched.h>

namespace Filesystem
{
namespace TTY
{
class PTS : public TTY
{
public:
    PTS(PTY* pty);

    virtual int ioctl(unsigned long request, char* argp, void* cookie) override;

    virtual ssize_t read(uint8_t* buffer, size_t count, void* cookie) override;
    virtual ssize_t write(uint8_t* buffer, size_t count, void* cookie) override;

private:
    PTY* pty;
};

PTS::PTS(PTY* pty)
{
    this->pty = pty;
}

int PTS::ioctl(unsigned long request, char* argp, void* cookie)
{
    return 0;
}

ssize_t PTS::read(uint8_t* buffer, size_t count, void* cookie)
{
    return pty->sread(buffer, count);
}
ssize_t PTS::write(uint8_t* buffer, size_t count, void* cookie)
{
    return pty->swrite(buffer, count);
}

} // namespace TTY
PTSFS::PTSFS()
{
    Filesystem::register_class(Filesystem::CHR, 134);
    this->root = new InitFS::Directory(0, 0, 0755);
}

bool PTSFS::mount(Superblock* sb)
{
    sb->root = libcxx::intrusive_ptr<Inode>(this->root);
    return true;
}

bool PTSFS::register_pty(TTY::PTY* pty)
{
    char name[128];
    TTY::PTS* pts = new TTY::PTS(pty);
    TTY::register_tty(134, pts);
    libcxx::sprintf(name, "pts%d", pty->index());
    this->root->mknod(name, S_IFCHR | 0644, mkdev(134, pty->index()));
    return true;
}
} // namespace Filesystem
