#pragma once

#include <types.h>

namespace Boot
{
struct info {
    const char* cmdline;
    void* architecture_data;
    addr_t kernel_start;
    addr_t kernel_end;
    addr_t initrd_start;
    addr_t initrd_end;
    addr_t highest;
};
}  // namespace Boot
