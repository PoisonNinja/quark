#pragma once

#include <lib/rb.h>
#include <lib/utility.h>

namespace Memory
{
class vmregion
{
public:
    vmregion(addr_t start, size_t size);
    vmregion(vmregion& other);

    libcxx::rbnode<vmregion> node;

    bool operator==(const vmregion& b) const;
    bool operator!=(const vmregion& b) const;
    bool operator<(const vmregion& b) const;
    bool operator>(const vmregion& b) const;

    addr_t start() const;
    addr_t end() const;
    size_t size() const;

private:
    addr_t _start;
    size_t _size;

    // Additional metadata to speed up accesses
    size_t largest_subgap; // Same idea as Linux

    friend class vma;
};

class vma
{
public:
    vma(addr_t lower_bound, addr_t upper_bound);
    vma(vma& other);
    ~vma();

    bool add_vmregion(addr_t start, size_t size);
    libcxx::pair<bool, addr_t> locate_range(addr_t hint, size_t size);
    libcxx::pair<bool, addr_t> allocate(addr_t hint, size_t size);
    void free(addr_t addr, size_t size);
    const vmregion* find(addr_t addr);

    void reset();

private:
    addr_t lower_bound, upper_bound;
    addr_t highest_mapped;

    vmregion *lowest, *highest;

    void traverse();
    void calculate_largest_subgap(vmregion* section);

    libcxx::rbtree<vmregion, &vmregion::node> sections;
};
} // namespace Memory
