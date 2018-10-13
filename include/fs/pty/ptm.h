#pragma once

#include <fs/inode.h>

namespace Filesystem
{
class PTMX : public KDevice
{
public:
    PTMX();
    virtual ~PTMX();
    virtual Pair<int, void*> open(const char* name) override;

    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset) override;

private:
    size_t next_pty_number;
};
} // namespace Filesystem
