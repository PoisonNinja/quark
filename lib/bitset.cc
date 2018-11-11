#include <lib/bitset.h>
#include <lib/string.h>

Bitset::Bitset(size_t size, uint8_t initial)
{
    this->allocated = Math::div_round_up(size, 8);
    this->bitset    = new uint8_t[this->allocated];
    libcxx::memset(this->bitset, initial, this->allocated);
}

Bitset::Bitset(size_t size)
    : Bitset(size, bitset_empty)
{
    // Delegated to other constructor to handle initialization
}

Bitset::~Bitset()
{
    delete[] this->bitset;
}

void Bitset::set(size_t index)
{
    this->bitset[index / 8] |= (1 << (index % 8));
}

void Bitset::unset(size_t index)
{
    this->bitset[index / 8] &= ~(1 << (index % 8));
}

bool Bitset::test(size_t index)
{
    return (bitset[index / 8] & (1 << (index % 8))) ? true : false;
}