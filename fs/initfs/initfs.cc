#include <fs/initfs/initfs.h>

namespace Filesystem
{
namespace InitFS
{
Directory::Directory(ino_t ino, dev_t dev, mode_t mode)
{
    this->ino = ino;
    this->dev = dev;
    this->mode = mode;
}

Directory::~Directory()
{
}

Ref<Inode> Directory::open(const char* name, int flags, mode_t mode)
{
    return Ref<Inode>(nullptr);
}
}
}
