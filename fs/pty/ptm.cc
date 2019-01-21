#include <fs/dev.h>
#include <fs/pty/ptm.h>
#include <fs/pty/pts.h>
#include <fs/pty/pty.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>

namespace filesystem
{
namespace tty
{
ptmx::ptmx()
    : next_pty_number(0)
{
}

ptmx::~ptmx()
{
}

libcxx::pair<int, void*> ptmx::open(const char* name)
{
    // TODO: Perhaps have PTSFS generate this?
    pty* p = new pty(next_pty_number++);
    static_cast<ptsfs*>(drivers::get("ptsfs"))->register_pty(p);
    return libcxx::pair<int, void*>(0, p);
}

int ptmx::ioctl(unsigned long request, char* argp, void* cookie)
{
    *reinterpret_cast<int*>(argp) = static_cast<pty*>(cookie)->index();
    return 0;
}

ssize_t ptmx::read(uint8_t* buffer, size_t count, void* cookie)
{
    return static_cast<pty*>(cookie)->mread(buffer, count);
}

ssize_t ptmx::write(const uint8_t* buffer, size_t count, void* cookie)
{
    return static_cast<pty*>(cookie)->mwrite(buffer, count);
}

namespace
{
int init()
{
    ptmx* p = new ptmx();
    log::printk(log::log_level::INFO, "Registering ptmx character device\n");
    filesystem::register_class(filesystem::CHR, 5);
    filesystem::tty::register_tty(5, p);
    return 0;
}
FS_INITCALL(init);
} // namespace
} // namespace tty
} // namespace filesystem
