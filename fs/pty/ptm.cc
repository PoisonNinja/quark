#include <fs/pty/ptm.h>
#include <fs/stat.h>
#include <kernel.h>

namespace Filesystem
{
namespace
{
}

PTM::PTM(ino_t ino, dev_t dev, mode_t mode)
{
    this->ino = (ino) ? ino : reinterpret_cast<ino_t>(this);
    this->dev = (dev) ? dev : reinterpret_cast<dev_t>(this);
    this->mode = mode | S_IFCHR;
    Log::printk(Log::LogLevel::INFO, "PTM created\n");
}

PTM::~PTM()
{
}

ssize_t PTM::read(uint8_t* buffer, size_t count, off_t offset)
{
    Log::printk(Log::LogLevel::INFO, "PTM read\n");
    return 0;
}

ssize_t PTM::write(uint8_t* buffer, size_t count, off_t offset)
{
    Log::printk(Log::LogLevel::INFO, "PTM read\n");
    return 0;
}

PTMX::PTMX(ino_t ino, dev_t dev, mode_t mode)
{
    this->ino = (ino) ? ino : reinterpret_cast<ino_t>(this);
    this->dev = (dev) ? dev : reinterpret_cast<dev_t>(this);
    this->mode = mode | S_IFCHR;
    this->flags = inode_factory;
    this->next_pty_number = 0;
}

PTMX::~PTMX()
{
}

Ref<Inode> PTMX::open(const char* name, int flags, mode_t mode)
{
    Log::printk(Log::LogLevel::INFO, "Creating PTMX\n");
    return Ref<PTM>(new PTM(0, 0, 0));
}
}  // namespace Filesystem