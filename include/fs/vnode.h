#pragma once

#include <fs/inode.h>

namespace Filesystem
{
class Vnode
{
public:
    ino_t ino;
    dev_t dev;
    mode_t mode;
};
}
