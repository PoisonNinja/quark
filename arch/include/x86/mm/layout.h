#pragma once

#include <types.h>

constexpr addr_t HEAP_START = 0xFFFF880000000000;

constexpr addr_t STACK_START = 0xFFFFFE8000000000;

constexpr addr_t VALLOC_START = 0xFFFFFC0000000000;

constexpr addr_t USER_START = 0x400000;
constexpr addr_t USER_END = 0x7FFFFFFFFFFF;
