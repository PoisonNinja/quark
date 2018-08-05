#pragma once

#include <cstddef>
#include <cstdint>

#ifdef X86_64
#define BITS 64

typedef int64_t ssize_t;

typedef uint64_t addr_t;
#else
#define BITS 32

typedef int32_t ssize_t;

typedef uint32_t addr_t;
#endif
