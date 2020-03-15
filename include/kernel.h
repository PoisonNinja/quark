#pragma once

#include <kernel/assert.h>
#include <kernel/log.h>

namespace kernel
{
[[noreturn]] void panic(const char* message, ...);
} // namespace kernel
