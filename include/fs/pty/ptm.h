#pragma once

#include <fs/inode.h>

namespace Filesystem
{
class PTM : public BaseInode
{
public:
    PTM(ino_t ino, dev_t dev, mode_t mode);
    virtual ~PTM();
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset) override;
};

class PTMX : public BaseInode
{
public:
    PTMX(ino_t ino, dev_t dev, mode_t mode);
    virtual ~PTMX();
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode) override;

private:
    size_t next_pty_number;
};
}  // namespace Filesystem