#pragma once

#include <types.h>

namespace boot
{
struct info {
    const char* cmdline;
    void* architecture_data;
    addr_t kernel_start;
    addr_t kernel_end;
    addr_t initrd_start;
    addr_t initrd_end;
};
}  // namespace boot
