#include <fs/dev.h>
#include <fs/ftable.h>
#include <fs/pty/ptm.h>
#include <fs/pty/pts.h>
#include <fs/pty/pty.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>

namespace Filesystem
{
namespace TTY
{
PTMX::PTMX()
    : next_pty_number(0)
{
}

PTMX::~PTMX()
{
}

libcxx::pair<int, void*> PTMX::open(const char* name)
{
    // TODO: Perhaps have PTSFS generate this?
    PTY* pty = new PTY(next_pty_number++);
    static_cast<PTSFS*>(FTable::get("ptsfs"))->register_pty(pty);
    return libcxx::pair<int, void*>(0, pty);
}

int PTMX::ioctl(unsigned long request, char* argp, void* cookie)
{
    *reinterpret_cast<int*>(argp) = static_cast<PTY*>(cookie)->index();
    return 0;
}

ssize_t PTMX::read(uint8_t* buffer, size_t count, void* cookie)
{
    return static_cast<PTY*>(cookie)->mread(buffer, count);
}

ssize_t PTMX::write(uint8_t* buffer, size_t count, void* cookie)
{
    return static_cast<PTY*>(cookie)->mwrite(buffer, count);
}

namespace
{
int init()
{
    PTMX* ptmx = new PTMX();
    Log::printk(Log::LogLevel::INFO, "Registering PTMX character device\n");
    Filesystem::register_class(Filesystem::CHR, 5);
    Filesystem::TTY::register_tty(5, ptmx);
    return 0;
}
FS_INITCALL(init);
} // namespace
} // namespace TTY
} // namespace Filesystem
