#include <kernel.h>
#include <lib/algorithm.h>
#include <lib/rb.h>
#include <mm/virtual.h>
#include <mm/vma.h>

namespace Memory
{
vmregion::vmregion(addr_t start, size_t size)
    : prev(nullptr)
    , next(nullptr)
    , largest_subgap(0)
    , _start(start)
    , _size(size)
{
}

vmregion::vmregion(vmregion& other)
    : vmregion(other._start, other._size)
{
}

addr_t vmregion::start() const
{
    return _start;
}

addr_t vmregion::end() const
{
    return _start + _size;
}

size_t vmregion::size() const
{
    return _size;
}

bool vmregion::operator==(const vmregion& b)
{
    return (_start == b._start) && (_size == b._size);
}

bool vmregion::operator!=(const vmregion& b)
{
    return !(*this == b);
}

bool vmregion::operator<(const vmregion& b)
{
    return this->_start < b._start;
}

vma::vma(addr_t start, addr_t end)
    : start(start)
    , end(end)
    , highest(0)
{
}

vma::vma(vma& other)
    : vma(other.start, other.end)
{
}

vma::~vma()
{
    this->reset();
}

bool vma::add_vmregion(addr_t start, size_t size)
{
    start             = Memory::Virtual::align_down(start);
    size              = Memory::Virtual::align_up(size);
    vmregion* section = new vmregion(start, size);
    // TODO: Sanity checks for overlaps, exceeding bounds
    // Of course, that probably requires support from rbtree
    section->prev = this->calculate_prev(start);
    if (section->prev) {
        vmregion* tnext = const_cast<vmregion*>(section->prev->next);
        vmregion* tprev = const_cast<vmregion*>(section->prev);
        tprev->next     = section;
        section->next   = tnext;
    } else {
        if (this->sections.parent(const_cast<const vmregion*>(section))) {
            section->next = const_cast<const vmregion*>(section);
        } else {
            section->next = nullptr;
        }
    }
    if (section->next) {
        vmregion* tnext = const_cast<vmregion*>(section->next);
        tnext->prev     = section;
    }
    auto func = libcxx::bind(&vma::calculate_largest_subgap, this,
                             libcxx::placeholders::_1);
    if (section->end() > highest) {
        highest = section->end();
    }
    this->sections.insert(*section, func);
    return true;
}

bool vma::locate_range(addr_t& ret, addr_t hint, size_t size)
{
    const vmregion* curr = sections.get_root();
    if (!curr) {
        // Woot woot there's nothing allocated, we can do anything we want
        ret = Memory::Virtual::align_up(hint);
        return true;
    }
    addr_t gap_start, gap_end;
    if (curr->largest_subgap < size) {
        goto check_highest;
    }
    while (true) {
        gap_end = curr->start();
        // Alternatively, check if the hint is any good
        // TODO: Check
        // If it's not, we are free to place it anywhere
        // Basically a port of Linux's algorithm
        if (this->sections.left(curr)) {
            if (this->sections.left(curr)->largest_subgap >= size) {
                curr = this->sections.left(curr);
                continue;
            }
        }
        gap_start = curr->prev ? (curr->prev->end()) : this->start;
    check_current:
        if (gap_end >= this->start && gap_end > gap_start &&
            gap_end - gap_start >= size) {
            goto found;
        }
        if (this->sections.right(curr)) {
            if (this->sections.right(curr)->largest_subgap >= size) {
                curr = this->sections.right(curr);
                continue;
            }
        }
        while (true) {
            auto prev = curr;
            if (!this->sections.parent(curr))
                goto check_highest;
            curr = this->sections.parent(curr);
            if (prev == this->sections.left(curr)) {
                gap_start = curr->prev->end();
                gap_end   = curr->start();
                goto check_current;
            }
        }
    }
check_highest:
    gap_start = this->highest;
    gap_end   = this->end;

found:
    ret = gap_start;
    return true;
}

const vmregion* vma::calculate_prev(addr_t addr)
{
    const vmregion* curr = sections.get_root();
    const vmregion* prev = nullptr;
    while (curr) {
        if (curr->end() > addr) {
            curr = sections.left(curr);
        } else {
            prev = curr;
            curr = sections.right(curr);
        }
    }
    return prev;
}

void vma::calculate_largest_subgap(vmregion* section)
{
    size_t max = 0;
    if (section->prev) {
        max = section->start() - section->prev->end();
    } else {
        max = section->start() - this->start;
    }
    if (section->next) {
        addr_t gap = section->next->start() - section->end();
        if (gap > max)
            max = gap;
    } else {
        addr_t gap = this->end - section->end();
        if (gap > max)
            max = gap;
    }
    if (this->sections.left(const_cast<const vmregion*>(section))) {
        if (this->sections.left(const_cast<const vmregion*>(section))
                ->largest_subgap > max) {
            max = this->sections.left(const_cast<const vmregion*>(section))
                      ->largest_subgap;
        }
    }
    if (this->sections.right(const_cast<const vmregion*>(section))) {
        if (this->sections.right(const_cast<const vmregion*>(section))
                ->largest_subgap > max) {
            max = this->sections.right(const_cast<const vmregion*>(section))
                      ->largest_subgap;
        }
    }
    section->largest_subgap = max;
}

void vma::reset()
{
}
} // namespace Memory
