#pragma once

#include <fs/inode.h>

namespace Filesystem
{
namespace InitFS
{
class File : public BaseInode
{
public:
    File();
    virtual ~File();
};

class Directory : public BaseInode
{
public:
    Directory(ino_t ino, dev_t dev, mode_t mode);
    virtual ~Directory();
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode);
};
}
}
