#pragma once

#include <fs/dev.h>

namespace memory
{
namespace dma
{
class sglist;
}
} // namespace memory

namespace Filesystem
{
enum class BlockRequestType : int {
    READ,
    WRITE,
};

using sector_t = uint64_t;

struct BlockRequest {
    BlockRequestType command;
    sector_t start;
    sector_t num_sectors;
    memory::dma::sglist* sglist;
};

class BlockDevice
{
public:
    virtual ~BlockDevice();

    virtual bool request(BlockRequest* request) = 0;

    // Size of a sector
    virtual sector_t sector_size() = 0;
    // Max size of a individual scatter gather target
    virtual size_t sg_max_size() = 0;
    // Max # of scatter gather targets
    virtual size_t sg_max_count() = 0;
};

bool register_blockdev(dev_t major, BlockDevice* blkdev);

} // namespace Filesystem