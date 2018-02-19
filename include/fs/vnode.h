#pragma once

#include <fs/inode.h>
#include <lib/refcount.h>

namespace Filesystem
{
class Vnode : public RefcountBase
{
public:
    ino_t ino;
    dev_t dev;
    mode_t mode;

    Vnode(Ref<Inode> inode);
    Ref<Vnode> open(const char* name, int flags, mode_t mode);

private:
    Ref<Inode> inode;
};
}
