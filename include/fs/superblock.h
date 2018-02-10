#pragma once

#include <types.h>

namespace FS
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
