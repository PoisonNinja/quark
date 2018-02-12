#pragma once

#include <fs/inode.h>
#include <fs/vnode.h>

namespace Filesystem
{
class Descriptor
{
public:
    ino_t ino;
    dev_t dev;
    mode_t mode;

    Descriptor(Vnode* vnode);

private:
    Vnode* vnode;
};
}
