#pragma once

#include <lib/rb.h>
#include <lib/utility.h>
#include <lib/vector.h>

namespace memory
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
    class iterator
    {
    public:
        explicit iterator(vmregion* v)
            : current(v){};
        vmregion& operator*()
        {
            return *current;
        }
        vmregion* operator->()
        {
            return current;
        }
        bool operator==(iterator const& other) const
        {
            return this->current == other.current;
        }
        bool operator!=(iterator const& other) const
        {
            return !(*this == other);
        }
        iterator& operator++()
        {
            this->current = this->current->node.next;
            return *this;
        }
        iterator& operator--()
        {
            this->current = this->current->node.prev;
            return *this;
        }

    private:
        vmregion* current;
    };

    vma(addr_t lower_bound, addr_t upper_bound);
    vma(const vma& other);
    vma& operator=(const vma& other);
    ~vma();

    // Doesn't make sense to ever move a VMA
    vma(const vma&& other) = delete;
    vma& operator=(const vma&& other) = delete;

    void swap(vma& other)
    {
        libcxx::swap(lower_bound, other.lower_bound);
        libcxx::swap(upper_bound, other.upper_bound);
        libcxx::swap(highest_mapped, other.lower_bound);
        libcxx::swap(lowest, other.lowest);
        libcxx::swap(highest, other.highest);
    }

    bool add_vmregion(addr_t start, size_t size);
    libcxx::pair<bool, addr_t> locate_range(addr_t hint, size_t size);
    libcxx::pair<bool, addr_t> locate_range_reverse(addr_t hint, size_t size);
    libcxx::pair<bool, addr_t> allocate(addr_t hint, size_t size);
    libcxx::pair<bool, addr_t> allocate_reverse(addr_t hint, size_t size);
    libcxx::pair<int, libcxx::vector<libcxx::pair<addr_t, size_t>>>
    free(addr_t addr, size_t size);
    vmregion* find(addr_t addr);

    vma::iterator begin()
    {
        return vma::iterator(lowest);
    }

    vma::iterator end()
    {
        // Umm, this is probably wrong
        return vma::iterator(nullptr);
    }

    void reset();

private:
    int split(struct vmregion* region, addr_t addr);
    bool add_vmregion(vmregion* region);
    void remove_node(vmregion* node);

    addr_t lower_bound, upper_bound;
    addr_t highest_mapped;

    vmregion *lowest, *highest;

    void traverse();
    void calculate_largest_subgap(vmregion* section);

    libcxx::rbtree<vmregion, &vmregion::node> sections;
};
} // namespace memory
