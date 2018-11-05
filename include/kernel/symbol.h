#pragma once

#include <lib/utility.h>
#include <types.h>

namespace Symbols
{
libcxx::pair<const char*, size_t> resolve_addr_fuzzy(addr_t address);
const char* resolve_addr(addr_t address);
addr_t resolve_name(const char* name);

void load_symbol(libcxx::pair<const char*, addr_t> symbol);
} // namespace Symbols
