#pragma once

#include <types.h>

namespace Filesystem
{
class Inode;

class Superblock
{
public:
    Superblock();
    ~Superblock();
    dev_t dev;
    Inode *root;

    virtual Inode *alloc_inode();
    virtual int read_inode(Inode *);
    virtual int write_inode(Inode *);
};
}
