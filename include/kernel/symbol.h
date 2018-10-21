#pragma once

#include <lib/pair.h>
#include <string>
#include <types.h>

namespace Symbols
{
Pair<std::string, size_t> resolve_addr_fuzzy(addr_t address);
std::string resolve_addr(addr_t address);
addr_t resolve_name(const char* name);

void load_symbol(Pair<const char*, addr_t> symbol);
} // namespace Symbols