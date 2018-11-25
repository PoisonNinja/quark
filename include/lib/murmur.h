#pragma once

#include <types.h>

namespace libcxx
{
namespace murmur
{
uint32_t hash(const void* key, uint32_t len);
}
} // namespace libcxx
