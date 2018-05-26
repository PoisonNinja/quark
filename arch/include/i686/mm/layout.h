#pragma once

#include <types.h>

// Other slots:
// 1023 - Fractal mapping
// 1020 - Copy mapping (for fork)

constexpr addr_t HEAP_START = 0xFF400000;    // Slot 1021
constexpr addr_t STACK_START = 0xFF800000;   // Slot 1022
constexpr addr_t VALLOC_START = 0xFEC00000;  // Slot 1019
constexpr addr_t USER_START = 0x400000;      // 4 MiB
constexpr addr_t USER_END = 0xC0000000;      // 3 GB
