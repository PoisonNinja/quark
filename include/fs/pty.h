#pragma once

#include <fs/inode.h>

namespace Filesystem
{
class PTMX : public BaseInode
{
public:
    PTMX(ino_t ino, dev_t dev, mode_t mode);
    ~PTMX();
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode) override;
};

class PTS : public BaseInode
{
public:
    PTS(ino_t ino, dev_t dev, mode_t mode);
};
}  // namespace Filesystem