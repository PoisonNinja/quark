#pragma once

#include <fs/driver.h>
#include <fs/inode.h>

namespace Filesystem
{
class PTSFS : public Driver
{
public:
    PTSFS();
    ~PTSFS();

    bool mount(Superblock* sb) override;
    uint32_t flags() override;
};
}  // namespace Filesystem