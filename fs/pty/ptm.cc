#include <fs/dev.h>
#include <fs/pty/ptm.h>
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

Pair<int, void*> PTMX::open(const char* name)
{
    Log::printk(Log::LogLevel::INFO, "ptmx: Opening!\n");
    return Pair<int, void*>(0, (void*)0xDEADBEEF + next_pty_number++);
}

int PTMX::ioctl(unsigned long request, char* argp, void* cookie)
{
    *argp = 5;
    return 0;
}

ssize_t PTMX::read(uint8_t*, size_t count, void* cookie)
{
    Log::printk(Log::LogLevel::INFO, "ptmx: Cookie is %p!\n", cookie);
    return count;
}

ssize_t PTMX::write(uint8_t*, size_t count, void* cookie)
{
    Log::printk(Log::LogLevel::INFO, "ptmx: Cookie is %p!\n", cookie);
    return count;
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
