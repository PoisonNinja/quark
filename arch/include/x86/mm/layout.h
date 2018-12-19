#pragma once

#include <types.h>

/*
 * Memory locations of various kernel stuff
 * Slot refers to a entry in the top level page table
 */
#ifdef X86_64
// clang-format off
constexpr addr_t USER_START    = 0x400000;
constexpr addr_t USER_END      = 0x7FFFFFFFFFFF;
constexpr addr_t HEAP_START    = 0xFFFF880000000000; // Slot 256
constexpr addr_t VMALLOC_START = 0xFFFFFC0000000000; // Slot 504
constexpr addr_t VMALLOC_END   = 0xFFFFFC8000000000; // Slot 505
constexpr addr_t DMA_START     = 0xFFFFFC8000000000; // Slot 505
// 508 - Copy mapping (for fork)
constexpr addr_t STACK_START   = 0xFFFFFE8000000000; // Slot 509
// 510 - Fractal mapping
constexpr addr_t VMA = 0xFFFFFFFF80000000; // Slot 511, subslot 510
#else
constexpr addr_t USER_START    = 0x400000;   // 4 MiB
constexpr addr_t USER_END      = 0xC0000000; // 3 GB
constexpr addr_t VMALLOC_START = 0xD0000000; // Slot 832
constexpr addr_t VMALLOC_END   = 0xD8000000; // Slot 864
constexpr addr_t DMA_START     = 0xD8000000; // Slot 864
constexpr addr_t HEAP_START    = 0xE0000000; // Slot 896
// 1020 - Copy mapping (for fork)
constexpr addr_t STACK_START   = 0xFF800000; // Slot 1022
// 1023 - Fractal mapping
constexpr addr_t VMA = 0xC0000000;
// clang-format on
#endif
