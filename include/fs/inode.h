#pragma once

#include <types.h>

namespace FS
{
class Dentry;
class Inode
{
public:
    Inode();
    ~Inode();

    ino_t ino;
    dev_t dev;
    mode_t mode;

    size_t size;

    virtual int lookup(Dentry *);
};
}
