#include <fs/block.h>
#include <kernel.h>
#include <lib/math.h>
#include <lib/string.h>
#include <mm/dma.h>

namespace filesystem
{
class block_wrapper : public kdevice
{
public:
    block_wrapper(block_device* bd);
    ~block_wrapper();

    ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    ssize_t write(const uint8_t* buffer, size_t count, off_t offset) override;

private:
    // TODO: Request queue
    block_device* blkdev;
};

block_wrapper::block_wrapper(block_device* bd)
    : kdevice(BLK)
    , blkdev(bd)
{
}

block_wrapper::~block_wrapper()
{
}

ssize_t block_wrapper::read(uint8_t* buffer, size_t count, off_t offset)
{
    // TODO: Eventually implement a more intelligent scheduler
    size_t processed = 0;
    size_t current   = offset;
    size_t remaining = count;

    /*
     * Build sglist. It's possible that we may not get a sglist that's capable
     * of holding the entire buffer, and that's OK. We'll just keep on sending
     * disk requests until we get all the data.
     */
    while (processed < count) {
        size_t to_process = count - processed;
        if (current % this->blkdev->sector_size())
            to_process += this->blkdev->sector_size();
        auto sglist =
            memory::dma::make_sglist(this->blkdev->sg_max_count(),
                                     this->blkdev->sg_max_size(), to_process);
        filesystem::block_request request;
        request.command     = filesystem::block_request_type::READ;
        request.num_sectors = sglist->total_size / blkdev->sector_size();
        request.start = libcxx::round_down(current, blkdev->sector_size()) /
                        blkdev->sector_size();
        request.sglist = libcxx::move(sglist);

        if (blkdev->request(&request)) {
            for (auto& region : request.sglist->list) {
                size_t to_copy = 0;
                if (current >
                    libcxx::round_down(current, blkdev->sector_size())) {
                    size_t distance =
                        current -
                        libcxx::round_down(current, blkdev->sector_size());
                    to_copy = region.size - distance;
                    if (remaining < to_copy) {
                        to_copy = remaining;
                    }
                    libcxx::memcpy(
                        buffer + processed,
                        reinterpret_cast<void*>(region.virtual_base + distance),
                        to_copy);
                } else {
                    to_copy = region.size;
                    if (remaining < to_copy) {
                        to_copy = remaining;
                    }
                    libcxx::memcpy(buffer + processed,
                                   reinterpret_cast<void*>(region.virtual_base),
                                   to_copy);
                }
                processed += to_copy;
                current += to_copy;
                remaining -= to_copy;
            }
        } else {
            log::printk(log::log_level::ERROR, "block: Request failed\n");
            return 0;
        }
        // TODO: We should probably free the sglist
    }
    return count;
}

ssize_t block_wrapper::write(const uint8_t* buffer, size_t count, off_t offset)
{
    // TODO: Eventually implement a more intelligent scheduler
    return 0;
}

block_device::~block_device()
{
}

bool register_blockdev(dev_t major, block_device* blkdev)
{
    block_wrapper* bw = new block_wrapper(blkdev);
    register_kdevice(BLK, major, bw);
    return true;
}
} // namespace filesystem
