#pragma once

#include <kernel/log.h>

namespace Kernel
{
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

void __attribute__((noreturn)) panic(const char* message, ...);
}
