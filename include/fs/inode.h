#pragma once

#include <lib/refcount.h>
#include <types.h>

namespace Filesystem
{
class Inode : public RefcountBase
{
public:
    ino_t ino;
    dev_t dev;
    mode_t mode;

    virtual ~Inode(){};
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode) = 0;
};

class BaseInode : public Inode
{
public:
    BaseInode();
    virtual ~BaseInode();
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode);
};
}
