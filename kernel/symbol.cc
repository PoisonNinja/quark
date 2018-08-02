#include <kernel.h>
#include <kernel/symbol.h>
#include <lib/hashmap.h>
#include <lib/list.h>
#include <lib/murmur.h>
#include <lib/string.h>

namespace
{
constexpr size_t table_size = 1024;

// A wrapper for char*
struct StringKey {
    StringKey(const char* s) : value(s){};
    const char* value;
    bool operator==(const struct StringKey& other)
    {
        return !String::strcmp(this->value, other.value);
    }
    bool operator!=(const struct StringKey& other)
    {
        return !(*this == other);
    }
};

struct AddressHash {
    unsigned long operator()(const addr_t key)
    {
        return key % table_size;
    }
};

struct NameHash {
    unsigned long operator()(const StringKey& k)
    {
        return Murmur::hash(k.value, String::strlen(k.value)) % table_size;
    }
};

Hashmap<StringKey, addr_t, table_size, NameHash> name_to_address_hash;
Hashmap<addr_t, const char*, table_size, AddressHash> address_to_name_hash;

struct Symbol {
    addr_t address;
    char* name;
    Node<Symbol> node;
};
List<Symbol, &Symbol::node> symbols;

bool ready = false;
}  // namespace

namespace Symbols
{
Pair<const char*, size_t> resolve_addr_fuzzy(addr_t address)
{
    if (!ready) {
        return primitive_resolve_addr(address);
    }
    size_t best = ~0;
    const char* ret = nullptr;
    for (auto& s : symbols) {
        if (s.address <= address) {
            size_t temp = address - s.address;
            if (temp < best) {
                ret = s.name;
                best = temp;
            }
        }
    }
    return make_pair(ret, best);
}

const char* resolve_addr(addr_t address)
{
    if (!ready) {
        return primitive_resolve_addr(address).first;
    }
    const char* name = nullptr;
    if (!address_to_name_hash.get(address, name)) {
        return nullptr;
    }
    return name;
}
addr_t resolve_name(const char* name)
{
    if (!ready) {
        return 0;
    }
    addr_t address = 0;
    if (!name_to_address_hash.get(name, address)) {
        return 0;
    }
    return address;
}

void load_symbol(Pair<const char*, addr_t> symbol)
{
    address_to_name_hash.put(symbol.second, symbol.first);
    name_to_address_hash.put(symbol.first, symbol.second);
    Symbol* s = new Symbol;
    s->address = symbol.second;
    s->name = new char[String::strlen(symbol.first) + 1];
    String::strcpy(s->name, symbol.first);
    symbols.push_back(*s);
    ready = true;
}
}  // namespace Symbols