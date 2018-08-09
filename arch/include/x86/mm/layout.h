#pragma once

#include <types.h>

/*
 * Memory locations of various kernel stuff
 * Slot refers to a entry in the top level page table
 */
#ifdef X86_64
constexpr addr_t HEAP_START = 0xFFFF880000000000;
constexpr addr_t STACK_START = 0xFFFFFE8000000000;
constexpr addr_t VALLOC_START = 0xFFFFFC0000000000;
constexpr addr_t USER_START = 0x400000;
constexpr addr_t USER_END = 0x7FFFFFFFFFFF;
constexpr addr_t VMA = 0xFFFFFFFF80000000;
#else
// Other slots:
// 1023 - Fractal mapping
// 1020 - Copy mapping (for fork)

constexpr addr_t HEAP_START = 0xFF400000;    // Slot 1021
constexpr addr_t STACK_START = 0xFF800000;   // Slot 1022
constexpr addr_t VALLOC_START = 0xFEC00000;  // Slot 1019
constexpr addr_t USER_START = 0x400000;      // 4 MiB
constexpr addr_t USER_END = 0xC0000000;      // 3 GB
constexpr addr_t VMA = 0xC0000000;
#endif
