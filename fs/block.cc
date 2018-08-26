#include <fs/block.h>
#include <kernel.h>
#include <lib/math.h>
#include <lib/string.h>
#include <mm/dma.h>

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
    // TODO: Request queue
    BlockDevice* blkdev;
};

BlockWrapper::BlockWrapper(BlockDevice* bd)
    : KDevice(BLK)
    , blkdev(bd)
{
}

BlockWrapper::~BlockWrapper()
{
}

ssize_t BlockWrapper::read(uint8_t* buffer, size_t count, off_t offset)
{
    // TODO: Eventually implement a more intelligent scheduler
    size_t processed = 0;
    size_t current   = offset;

    /*
     * Build sglist. It's possible that we may not get a sglist that's capable
     * of holding the entire buffer, and that's OK. We'll just keep on sending
     * disk requests until we get all the data.
     */
    while (processed < count) {
        Memory::DMA::SGList* sglist = Memory::DMA::build_sglist(
            this->blkdev->sg_max_count(), this->blkdev->sg_max_size(), count);
        Log::printk(Log::LogLevel::INFO,
                    "block: SGList contains %p bytes in %llX regions\n",
                    sglist->total_size, sglist->num_regions);
        Filesystem::BlockRequest request;
        request.command     = Filesystem::BlockRequestType::READ;
        request.num_sectors = sglist->total_size / blkdev->sector_size();
        request.start       = Math::round_down(current, blkdev->sector_size()) /
                        blkdev->sector_size();
        request.sglist = sglist;
        Log::printk(Log::LogLevel::INFO, "block: 0x%zX sectors, 0x%zX start\n",
                    request.num_sectors, request.start);

        if (blkdev->request(&request)) {
            for (auto& region : sglist->list) {
                if (current >
                    Math::round_down(current, blkdev->sector_size())) {
                    Log::printk(Log::LogLevel::DEBUG,
                                "block: Unaligned disk read :(\n");
                    size_t distance =
                        current -
                        Math::round_down(current, blkdev->sector_size());
                    Log::printk(Log::LogLevel::DEBUG,
                                "block: Block offset 0x%zX\n", distance);
                    String::memcpy(
                        buffer + processed,
                        reinterpret_cast<void*>(region.virtual_base + distance),
                        region.size - distance);
                    processed += region.size - distance;
                } else {
                    Log::printk(Log::LogLevel::DEBUG,
                                "block: Aligned disk read :)\n");
                    String::memcpy(buffer + processed,
                                   reinterpret_cast<void*>(region.virtual_base),
                                   region.size);
                    processed += region.size;
                }
            }
        } else {
            Log::printk(Log::LogLevel::ERROR, "block: Request failed\n");
            return 0;
        }

        // TODO: We should probably free the sglist
    }
    return count;
}

ssize_t BlockWrapper::write(uint8_t* buffer, size_t count, off_t offset)
{
    // TODO: Eventually implement a more intelligent scheduler
    return 0;
}

BlockDevice::~BlockDevice()
{
}

bool register_blockdev(dev_t major, BlockDevice* blkdev)
{
    BlockWrapper* bw = new BlockWrapper(blkdev);
    register_kdevice(BLK, major, bw);
    return true;
}
} // namespace Filesystem
