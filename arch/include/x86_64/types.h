#pragma once

#include <cstddef>
#include <cstdint>

#ifdef X86_64
#define BITS 64

using ssize_t = int64_t;

using addr_t = uint64_t;
#else
#define BITS 32

using ssize_t = int32_t;

using addr_t = uint32_t;
#endif
