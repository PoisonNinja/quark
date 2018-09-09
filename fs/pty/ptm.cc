#include <fs/dev.h>
#include <fs/pty/ptm.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>

namespace Filesystem
{
PTMX::PTMX()
    : KDevice(Filesystem::CHR)
{
}

PTMX::~PTMX()
{
}

int PTMX::open(const char* name, dev_t dev)
{
    Log::printk(Log::LogLevel::INFO, "ptmx: Opening!\n");
    return 0;
}

ssize_t PTMX::read(uint8_t*, size_t count, off_t)
{
    return count;
}

ssize_t PTMX::write(uint8_t*, size_t count, off_t)
{
    return count;
}

namespace
{
int init()
{
    PTMX* ptmx = new PTMX();
    Log::printk(Log::LogLevel::INFO, "Registering PTMX character device\n");
    Filesystem::register_class(Filesystem::CHR, 5);
    Filesystem::register_kdevice(Filesystem::CHR, 5, ptmx);
    return 0;
}
FS_INITCALL(init);
} // namespace
} // namespace Filesystem
