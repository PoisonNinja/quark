#pragma once

#include <fs/inode.h>
#include <fs/vnode.h>
#include <lib/refcount.h>

namespace Filesystem
{
class Descriptor : public RefcountBase
{
public:
    ino_t ino;
    dev_t dev;
    mode_t mode;

    Descriptor(Ref<Vnode> vnode);
    Ref<Descriptor> open(const char* name, int flags, mode_t mode);

private:
    Ref<Vnode> vnode;
};
}
