#include <kernel.h>
#include <lib/algorithm.h>
#include <lib/rb.h>
#include <mm/virtual.h>
#include <mm/vma.h>

namespace Memory
{
vmregion::vmregion(addr_t start, size_t size)
    : largest_subgap(0)
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

bool vmregion::operator==(const vmregion& b) const
{
    return (_start == b._start) && (_size == b._size);
}

bool vmregion::operator!=(const vmregion& b) const
{
    return !(*this == b);
}

bool vmregion::operator<(const vmregion& b) const
{
    return this->_start < b._start;
}

bool vmregion::operator>(const vmregion& b) const
{
    return this->_start > b._start;
}

vma::vma(addr_t start, addr_t end)
    : start(start)
    , end(end)
    , highest(start)
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
    auto func = libcxx::bind(&vma::calculate_largest_subgap, this,
                             libcxx::placeholders::_1);
    if (section->end() > highest) {
        highest = section->end();
    }
    this->sections.insert(*section, func);
    return true;
}

libcxx::pair<bool, addr_t> vma::locate_range(addr_t hint, size_t size)
{
    const vmregion* curr = sections.get_root();
    addr_t high_limit    = this->end - size;
    addr_t low_limit     = this->start;
    addr_t ret           = 0;
    if (!curr) {
        goto check_highest;
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
        gap_start = this->sections.prev(curr)
                        ? (this->sections.prev(curr)->end())
                        : low_limit;
    check_current:
        if (gap_end >= low_limit && gap_end > gap_start &&
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
                gap_start = this->sections.prev(curr)->end();
                gap_end   = curr->start();
                goto check_current;
            }
        }
    }
check_highest:
    gap_start = this->highest;
    gap_end   = high_limit;

found:
    ret = gap_start;
    return libcxx::make_pair(true, ret);
}

void vma::free(addr_t addr, size_t size)
{
    // **** your const :P
    vmregion* node = const_cast<vmregion*>(find(addr));
    node           = this->sections.remove(*node);
    delete node;
}

const vmregion* vma::find(addr_t addr)
{
    const vmregion* curr = sections.get_root();
    const vmregion* ret  = nullptr;
    while (curr) {
        if (curr->end() > addr) {
            ret = curr;
            if (ret->start() <= addr)
                break;
            curr = this->sections.left(curr);
        } else
            curr = this->sections.right(curr);
    }
    return ret;
}

void vma::calculate_largest_subgap(vmregion* section)
{
    size_t max = 0;
    if (this->sections.prev(const_cast<const vmregion*>(section))) {
        max = section->start() -
              this->sections.prev(const_cast<const vmregion*>(section))->end();
    } else {
        max = section->start() - this->start;
    }
    if (this->sections.next(const_cast<const vmregion*>(section))) {
        addr_t gap =
            this->sections.next(const_cast<const vmregion*>(section))->start() -
            section->end();
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

libcxx::pair<bool, addr_t> vma::allocate(addr_t hint, size_t size)
{
    auto res = locate_range(hint, size);
    if (res.first) {
        add_vmregion(res.second, size);
    }
    return res;
}

void vma::reset()
{
}
} // namespace Memory
