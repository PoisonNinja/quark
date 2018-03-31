#pragma once

#include <arch/mm/layout.h>
#include <types.h>

namespace Memory
{
namespace Physical
{
addr_t* const STACK = reinterpret_cast<addr_t*>(STACK_START);
}
}
