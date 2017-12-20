#pragma once

#include <types.h>

namespace Boot
{
struct info {
    const char* cmdline;
    void* architecture_data;
};
}  // namespace Boot
