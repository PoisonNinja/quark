#pragma once

#include <types.h>

constexpr addr_t HEAP_START = 0xFFFF880000000000;

constexpr addr_t STACK_START = 0xFF800000;

constexpr addr_t USER_START = 0x400000;
constexpr addr_t USER_END = 0x7FFFFFFFFFFF;
