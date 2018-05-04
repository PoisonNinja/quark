#pragma once

#include <types.h>

constexpr addr_t HEAP_START = 0xFF400000;  // Slot 1021

constexpr addr_t STACK_START = 0xFF800000;  // Slot 1022

constexpr addr_t VALLOC_START = 0xFF000000;  // Slot 1020

constexpr addr_t USER_START = 0x400000;
constexpr addr_t USER_END = 0x7FC00000;
