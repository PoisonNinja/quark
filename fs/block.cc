#include <fs/block.h>

namespace Filesystem
{
class BlockWrapper : public KDevice
{
public:
    BlockWrapper(BlockDevice* bd);
    ~BlockWrapper();

    ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    ssize_t write(uint8_t* buffer, size_t count, off_t offset) override;

private:
    BlockDevice* blkdev;
};

BlockWrapper::BlockWrapper(BlockDevice* bd) : KDevice(BLK), blkdev(bd)
{
}

BlockWrapper::~BlockWrapper()
{
}

ssize_t BlockWrapper::read(uint8_t* buffer, size_t count, off_t offset)
{
    // TODO: Eventually implement a more intelligent scheduler
    return blkdev->read(buffer, count, offset);
}

ssize_t BlockWrapper::write(uint8_t* buffer, size_t count, off_t offset)
{
    // TODO: Eventually implement a more intelligent scheduler
    return blkdev->write(buffer, count, offset);
}

bool register_blockdev(dev_t major, BlockDevice* blkdev)
{
    BlockWrapper* bw = new BlockWrapper(blkdev);
    register_kdevice(BLK, major, bw);
}
}  // namespace Filesystem
