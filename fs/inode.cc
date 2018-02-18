#include <fs/inode.h>

namespace Filesystem
{
BaseInode::BaseInode()
{
    ino = 0;
    dev = 0;
    mode = 0;
}

BaseInode::~BaseInode()
{
}

Ref<Inode> BaseInode::open(const char*, int, mode_t)
{
    return Ref<Inode>(nullptr);
}
}
