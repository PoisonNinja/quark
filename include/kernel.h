#pragma once

#include <kernel/log.h>

[[noreturn]] void __assert(const char* message, const char* file, int line);

namespace kernel
{
[[noreturn]] void panic(const char* message, ...);

#ifdef QUARK_DEBUG
#define assert(EX) (void)((EX) || (__assert(#EX, __FILE__, __LINE__), false))
#else
#define assert(EX)
#endif
} // namespace kernel
