#include <kernel.h>
#include <kernel/symbol.h>
#include <lib/hashmap.h>
#include <lib/list.h>
#include <lib/murmur.h>
#include <lib/string.h>
#include <unordered_map>

namespace
{
std::unordered_map<std::string, addr_t> name_to_address_hash;
} // namespace

namespace Symbols
{
Pair<std::string, size_t> resolve_addr_fuzzy(addr_t address)
{
    size_t best = ~0;
    std::string ret;
    for (auto& s : name_to_address_hash) {
        if (s.second <= address) {
            size_t temp = address - s.second;
            if (temp < best) {
                ret  = s.first;
                best = temp;
            }
        }
    }
    return Pair<std::string, size_t>(ret, best);
}

addr_t resolve_name(const char* name)
{
    if (name_to_address_hash.find(name) != name_to_address_hash.end()) {
        return name_to_address_hash[name];
    }
    return 0;
}

void load_symbol(Pair<const char*, addr_t> symbol)
{
    name_to_address_hash[symbol.first] = symbol.second;
}

extern void arch_init();

void init()
{
    arch_init();
}
} // namespace Symbols