#include <lib/bitset.h>
#include <lib/string.h>

namespace libcxx
{
bitset::reference::reference(uint8_t* val, size_t offset)
    : val(val)
    , offset(offset)
{
}

bitset::reference::~reference()
{
    // Nothing
}

bitset::reference::operator bool() const
{
    return this->value();
}

bool bitset::reference::operator~() const
{
    return !(this->value());
}

bitset::reference& bitset::reference::operator=(bool x)
{
    if (x) {
        *(this->val) |= (1 << (this->offset % 8));
    } else {
        *(this->val) &= ~(1 << (this->offset % 8));
    }
    return *this;
}

bitset::reference& bitset::reference::operator=(const reference& x)
{
    bool set = x.value();
    if (set) {
        *(this->val) |= (1 << (this->offset % 8));
    } else {
        *(this->val) &= ~(1 << (this->offset % 8));
    }
    return *this;
}

bitset::reference& bitset::reference::flip()
{
    *this->val ^= (1 << (this->offset % 8));
    return *this;
}

bool bitset::reference::value() const
{
    return (*(this->val) & (1 << (this->offset % 8))) ? true : false;
}

bitset::bitset(size_t size, bool initial)
{
    this->allocated = Math::div_round_up(size, 8);
    this->data      = new uint8_t[this->allocated];
    uint8_t val     = (initial) ? 0xFF : 0x00;
    libcxx::memset(this->data, val, this->allocated);
}

bitset::bitset(size_t size)
    : bitset(size, false)
{
    // Delegated to other constructor to handle initialization
}

bitset::~bitset()
{
    delete[] this->data;
}

void bitset::set(size_t index)
{
    this->data[index / 8] |= (1 << (index % 8));
}

void bitset::unset(size_t index)
{
    this->data[index / 8] &= ~(1 << (index % 8));
}

bool bitset::test(size_t index) const
{
    return (data[index / 8] & (1 << (index % 8))) ? true : false;
}

bitset::reference bitset::operator[](size_t index)
{
    return reference(&this->data[index / 8], index % 8);
}
} // namespace libcxx
