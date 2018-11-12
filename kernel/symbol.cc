#include <kernel.h>
#include <kernel/symbol.h>
#include <lib/list.h>
#include <lib/murmur.h>
#include <lib/string.h>
#include <lib/unordered_map.h>

namespace
{
struct AddressHash {
    unsigned long operator()(const addr_t key)
    {
        return key;
    }
};

constexpr size_t symbol_size = 4096;

libcxx::unordered_map<libcxx::StringKey, addr_t, symbol_size,
                      libcxx::StringHash>
    name_to_address_hash;
libcxx::unordered_map<addr_t, const char*, symbol_size, AddressHash>
    address_to_name_hash;

struct Symbol {
    addr_t address;
    char* name;
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
                ret  = s.name;
                best = temp;
            }
        }
    }
    return libcxx::make_pair(ret, best);
}

const char* resolve_addr(addr_t address)
{
    const char* name = nullptr;
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
    s->name    = new char[libcxx::strlen(symbol.first) + 1];
    libcxx::strcpy(s->name, symbol.first);
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
