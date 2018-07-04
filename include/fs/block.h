#pragma once

#include <fs/dev.h>

namespace Filesystem
{
class BlockDevice
{
public:
    virtual ~BlockDevice();

    // A subset of Inode operations
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset) = 0;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset) = 0;

private:
    // TODO: Request queue
};

bool register_blockdev(dev_t major, BlockDevice* blkdev);

}  // namespace Filesystem