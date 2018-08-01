#pragma once

#include <lib/pair.h>
#include <types.h>

namespace Symbols
{
Pair<const char*, size_t> resolve_addr_fuzzy(addr_t address);
const char* resolve_addr(addr_t address);
addr_t resolve_name(const char* name);

// Before the mm subsystem is online
Pair<const char*, size_t> primitive_resolve_addr(addr_t address);

void load_symbol(Pair<const char*, addr_t> symbol);
}  // namespace Symbols