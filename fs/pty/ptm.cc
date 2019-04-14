#include <fs/dev.h>
#include <fs/pty/ptm.h>
#include <fs/pty/pts.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>

namespace filesystem
{
namespace tty
{
namespace
{
struct ptmx_metadata {
    struct tty_core* tty;
    size_t index;
};
} // namespace

ptm::ptm()
{
}

ssize_t ptm::write(const uint8_t* buffer, size_t count)
{
}

ptmx::ptmx(ptsfs* fs)
    : kdevice(filesystem::CHR)
    , fs(fs)
{
}

int ptmx::ioctl(unsigned long request, char* argp, void* cookie)
{
    *reinterpret_cast<int*>(argp) = static_cast<ptmx_metadata*>(cookie)->index;
    return 0;
}

libcxx::pair<int, void*> ptmx::open(const char* name)
{
    ptm* master                         = new ptm();
    ptmx_metadata* meta                 = new ptmx_metadata;
    libcxx::tie(meta->index, meta->tty) = this->fs->register_ptm(master);
    return libcxx::pair<int, void*>(0, meta);
}

ssize_t ptmx::read(uint8_t* buffer, size_t count, off_t offset, void* cookie)
{
    return static_cast<tty_core*>(cookie)->read(buffer, count, offset, cookie);
}

ssize_t ptmx::write(const uint8_t* buffer, size_t count, off_t offset,
                    void* cookie)
{
    return static_cast<tty_core*>(cookie)->write(buffer, count, offset, cookie);
}
} // namespace tty
} // namespace filesystem
