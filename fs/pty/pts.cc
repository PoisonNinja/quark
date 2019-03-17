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

namespace
{
constexpr int pty_major = 134;
}

namespace filesystem
{
namespace tty
{
class PTS : public tty
{
public:
    PTS(pty* p);

    virtual int ioctl(unsigned long request, char* argp, void* cookie) override;

    virtual int poll(filesystem::poll_register_func_t& callback,
                     void* cookie) override;
    virtual ssize_t read(uint8_t* buffer, size_t count, void* cookie) override;
    virtual ssize_t write(const uint8_t* buffer, size_t count,
                          void* cookie) override;

private:
    pty* p;
};

PTS::PTS(pty* p)
{
    this->p = p;
}

int PTS::ioctl(unsigned long request, char* argp, void* cookie)
{
    return 0;
}

int PTS::poll(filesystem::poll_register_func_t& callback, void* cookie)
{
    return p->spoll(callback);
}

ssize_t PTS::read(uint8_t* buffer, size_t count, void* cookie)
{
    return p->sread(buffer, count);
}
ssize_t PTS::write(const uint8_t* buffer, size_t count, void* cookie)
{
    return p->swrite(buffer, count);
}

} // namespace tty
ptsfs::ptsfs()
{
    filesystem::register_class(filesystem::CHR, pty_major);
    this->root = new tmpfs::directory(0, 0, 0755);
}

bool ptsfs::mount(superblock* sb)
{
    sb->root = libcxx::intrusive_ptr<inode>(this->root);
    return true;
}

bool ptsfs::register_pty(tty::pty* pty)
{
    char name[128];
    tty::PTS* pts = new tty::PTS(pty);
    tty::register_tty(pty_major, pts);
    libcxx::sprintf(name, "pts%d", pty->index());
    this->root->mknod(name, S_IFCHR | 0644, mkdev(pty_major, pty->index()));
    return true;
}
} // namespace filesystem
