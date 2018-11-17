#include <kernel.h>
#include <kernel/symbol.h>
#include <lib/list.h>
#include <lib/murmur.h>
#include <lib/string.h>
#include <lib/unordered_map.h>

namespace
{

constexpr size_t symbol_size = 4096;

libcxx::unordered_map<libcxx::string, addr_t, symbol_size> name_to_address_hash;
libcxx::unordered_map<addr_t, libcxx::string, symbol_size> address_to_name_hash;

struct Symbol {
    addr_t address;
    libcxx::string name;
    libcxx::node<Symbol> node;
};
libcxx::list<Symbol, &Symbol::node> symbols;
} // namespace

namespace Symbols
{
libcxx::pair<const char*, size_t> resolve_addr_fuzzy(addr_t address)
{
    size_t best     = ~0;
    const char* ret = nullptr;
    for (auto& s : symbols) {
        if (s.address <= address) {
            size_t temp = address - s.address;
            if (temp < best) {
                ret  = s.name.c_str();
                best = temp;
            }
        }
    }
    return libcxx::make_pair(ret, best);
}

libcxx::string resolve_addr(addr_t address)
{
    libcxx::string name;
    if (!address_to_name_hash.at(address, name)) {
        return nullptr;
    }
    return name;
}
addr_t resolve_name(const char* name)
{
    addr_t address = 0;
    if (!name_to_address_hash.at(name, address)) {
        return 0;
    }
    return address;
}

void load_symbol(libcxx::pair<const char*, addr_t> symbol)
{
    Symbol* s  = new Symbol;
    s->address = symbol.second;
    s->name    = libcxx::move(libcxx::string(symbol.first));
    symbols.push_back(*s);
    address_to_name_hash.insert(symbol.second, s->name);
    name_to_address_hash.insert(s->name, symbol.second);
}

extern void arch_init();

void init()
{
    arch_init();
}
} // namespace Symbols
