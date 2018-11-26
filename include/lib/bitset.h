#pragma once

#include <lib/math.h>
#include <types.h>

namespace libcxx
{
class bitset
{
public:
    /*
     * Proxy class for accessing bitset using []
     */
    class reference
    {
    public:
        ~reference();
        operator bool() const;
        bool operator~() const;
        reference& operator=(bool x);
        reference& operator=(const reference& x);
        reference& flip();

    private:
        friend class bitset;
        reference(uint8_t* val, size_t offset);
        bool value() const;
        uint8_t* val;
        size_t offset;
    };

    bitset(size_t size, bool initial);
    bitset(size_t size);
    ~bitset();

    void set(size_t index);
    void unset(size_t index);
    bool test(size_t index) const;

    reference operator[](size_t index);

private:
    size_t allocated;
    uint8_t* data;
};
} // namespace libcxx
