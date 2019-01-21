#pragma once

#include <lib/string.h>
#include <lib/utility.h>
#include <types.h>

namespace symbols
{
libcxx::pair<const char*, size_t> resolve_addr_fuzzy(addr_t address);
libcxx::string resolve_addr(addr_t address);
addr_t resolve_name(const char* name);

void load_symbol(libcxx::pair<const char*, addr_t> symbol);

void init();
} // namespace symbols
