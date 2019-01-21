#pragma once

#include <fs/dev.h>
#include <lib/memory.h>

namespace memory
{
namespace dma
{
class sglist;
}
} // namespace memory

namespace filesystem
{
enum class block_request_type : int {
    READ,
    WRITE,
};

using sector_t = uint64_t;

struct block_request {
    block_request_type command;
    sector_t start;
    sector_t num_sectors;
    libcxx::unique_ptr<memory::dma::sglist> sglist;
};

class block_device
{
public:
    virtual ~block_device();

    virtual bool request(block_request* request) = 0;

    // Size of a sector
    virtual sector_t sector_size() = 0;
    // Max size of a individual scatter gather target
    virtual size_t sg_max_size() = 0;
    // Max # of scatter gather targets
    virtual size_t sg_max_count() = 0;
};

bool register_blockdev(dev_t major, block_device* blkdev);

} // namespace filesystem
