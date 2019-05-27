#pragma once

#include <types.h>

/*
 * Memory locations of various kernel stuff
 * Slot refers to a entry in the top level page table
 */
// clang-format off
constexpr addr_t USER_START    = 0x400000;
constexpr addr_t USER_END      = 0x7FFFFFFFFFFF;
constexpr addr_t PHYS_START    = 0xFFFF800000000000; // Slot 256
constexpr addr_t HEAP_START    = 0xFFFFFA0000000000; // Slot 500
constexpr addr_t VMALLOC_START = 0xFFFFFC0000000000; // Slot 504
constexpr addr_t VMALLOC_END   = 0xFFFFFC8000000000; // Slot 505
constexpr addr_t DMA_START     = 0xFFFFFC8000000000; // Slot 505
constexpr addr_t DMA_END       = 0xFFFFFD0000000000; // Slot 506
// 508 - Copy mapping (for fork)
constexpr addr_t STACK_START   = 0xFFFFFE8000000000; // Slot 509
// 510 - Fractal mapping
constexpr addr_t VMA = 0xFFFFFFFF80000000; // Slot 511, subslot 510
