#pragma once

#include <fs/driver.h>
#include <fs/inode.h>

namespace Filesystem
{
class PTS : public Driver
{
public:
    PTS();
    ~PTS();

    bool mount(Superblock* sb) override;
    uint32_t flags() override;
};
}  // namespace Filesystem