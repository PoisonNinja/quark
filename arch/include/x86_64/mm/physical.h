#pragma once

#include <types.h>

namespace Memory
{
namespace Physical
{
addr_t* const STACK = reinterpret_cast<addr_t*>(0xFFFFFE8000000000);
}
}
