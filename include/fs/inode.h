#pragma once

#include <types.h>

namespace Filesystem
{
struct Dentry;
class Superblock;
class Inode
{
public:
    Inode();
    ~Inode();

    ino_t ino;
    dev_t dev;
    mode_t mode;

    size_t size;

    Superblock* superblock;

    virtual Inode* create(Dentry* dentry, int flags, mode_t mode);
    virtual int lookup(Dentry*);
};

namespace InodeCache
{
void init();
Inode* get(Superblock* superblock, ino_t ino);
void set(Inode* inode);
}
}
