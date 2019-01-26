#pragma once

#include <fs/driver.h>

namespace filesystem
{
namespace ext2
{
class driver : public filesystem::driver
{
public:
    driver();
    ~driver();
    bool mount(superblock* sb) override;
    uint32_t flags() override;
};
} // namespace ext2
} // namespace filesystem