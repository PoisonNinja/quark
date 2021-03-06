#include <kernel.h>
#include <lib/algorithm.h>
#include <lib/rb.h>
#include <mm/virtual.h>
#include <mm/vma.h>

namespace memory
{
vmregion::vmregion(addr_t start, size_t size)
    : _start(start)
    , _size(size)
    , largest_subgap(0)
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

vma::vma(addr_t lower_bound, addr_t upper_bound)
    : lower_bound(lower_bound)
    , upper_bound(upper_bound)
    , highest_mapped(lower_bound)
    , lowest(nullptr)
    , highest(nullptr)
{
}

vma::vma(const vma& other)
    : lower_bound(other.lower_bound)
    , upper_bound(other.upper_bound)
    , highest_mapped(other.lower_bound)
    , lowest(nullptr)
    , highest(nullptr)
{
    for (auto vmr = other.lowest; vmr != nullptr; vmr = other.tree.next(vmr)) {
        this->add_vmregion(vmr->start(), vmr->size());
    }
}

vma& vma::operator=(const vma& other)
{
    vma tmp(other);
    this->swap(tmp);
    return *this;
}

vma::~vma()
{
    this->reset();
}

bool vma::add_vmregion(vmregion* section)
{
    // TODO: Sanity checks for overlaps, exceeding bounds
    // Of course, that probably requires support from rbtree
    auto func = libcxx::bind(&vma::calculate_largest_subgap, this,
                             libcxx::placeholders::_1);
    if ((highest && section->end() > highest_mapped) || !highest) {
        // TODO: Maybe get rid of highest_mapped?
        highest_mapped = section->end();
        highest        = section;
    }
    if ((lowest && section->start() < lowest->start()) || !lowest) {
        lowest = section;
    }
    this->tree.insert(*section, func);
    return true;
}

bool vma::add_vmregion(addr_t start, size_t size)
{
    start             = memory::virt::align_down(start);
    size              = memory::virt::align_up(size);
    vmregion* section = new vmregion(start, size);
    return this->add_vmregion(section);
}

libcxx::optional<addr_t> vma::locate_range(addr_t hint, size_t size)
{
    vmregion* curr    = tree.get_root();
    addr_t high_limit = this->upper_bound - size;
    addr_t low_limit  = this->lower_bound;
    addr_t ret        = 0;
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
        if (gap_end >= low_limit && this->tree.left(curr)) {
            if (this->tree.left(curr)->largest_subgap >= size) {
                curr = this->tree.left(curr);
                continue;
            }
        }
        gap_start =
            this->tree.prev(curr) ? (this->tree.prev(curr)->end()) : low_limit;
    check_current:
        if (gap_end >= low_limit && gap_end > gap_start &&
            gap_end - gap_start >= size) {
            goto found;
        }
        if (this->tree.right(curr)) {
            if (this->tree.right(curr)->largest_subgap >= size) {
                curr = this->tree.right(curr);
                continue;
            }
        }
        while (true) {
            auto prev = curr;
            if (!this->tree.parent(curr))
                goto check_highest;
            curr = this->tree.parent(curr);
            if (prev == this->tree.left(curr)) {
                gap_start = this->tree.prev(curr)->end();
                gap_end   = curr->start();
                goto check_current;
            }
        }
    }
check_highest:
    gap_start = this->highest_mapped;
    gap_end   = high_limit;

found:
    ret = gap_start;
    return {ret};
}

libcxx::optional<addr_t> vma::locate_range_reverse(addr_t hint, size_t size)
{
    vmregion* curr    = tree.get_root();
    addr_t high_limit = this->upper_bound - size;
    addr_t low_limit  = this->lower_bound;
    addr_t gap_end    = this->upper_bound;
    addr_t gap_start  = this->highest->end();
    if (gap_start <= high_limit) {
        goto found_highest;
    }
    if (!curr || curr->largest_subgap < size) {
        // TODO: ENOMEM
        return libcxx::nullopt;
    }
    while (true) {
        gap_start = this->tree.prev(curr) ? this->tree.prev(curr)->end() : 0;
        if (gap_start <= high_limit && this->tree.right(curr)) {
            if (this->tree.right(curr)->largest_subgap >= size) {
                curr = this->tree.right(curr);
                continue;
            }
        }
    check_current:
        gap_end = curr->start();
        if (gap_end < low_limit) {
            // TODO: ENOMEM
            return libcxx::nullopt;
        }
        if (gap_start <= high_limit && gap_end > gap_start &&
            gap_end - gap_start >= size) {
            goto found;
        }
        if (this->tree.left(curr)) {
            if (this->tree.left(curr)->largest_subgap >= size) {
                curr = this->tree.left(curr);
                continue;
            }
        }
        while (true) {
            auto prev = curr;
            if (!this->tree.parent(curr)) {
                // TODO: ENOMEM
                return libcxx::nullopt;
            }
            curr = this->tree.parent(curr);
            if (prev == this->tree.right(curr)) {
                gap_start =
                    (this->tree.prev(curr)) ? this->tree.prev(curr)->end() : 0;
                gap_end = curr->start();
                goto check_current;
            }
        }
    }

found:
    if (gap_end > high_limit)
        gap_end = high_limit;

found_highest:
    gap_end -= size;
    gap_end = memory::virt::align_down(gap_end);
    return {gap_end};
}

int vma::split(vmregion* region, addr_t addr)
{
    /*
     * Very stupid algorithm, basically we remove the old element and
     * insert the two new ones back in
     */
    // Splitting at addr
    this->remove_node(region);

    // Adjust boundaries
    this->add_vmregion(region->start(), addr - region->start());

    region->_start = addr;
    region->_size -= addr - region->start();

    this->add_vmregion(region);
    return 0;
}

void vma::remove_node(vmregion* node)
{
    if (!node) {
        return;
    }
    auto func = libcxx::bind(&vma::calculate_largest_subgap, this,
                             libcxx::placeholders::_1);
    node      = this->tree.remove(*node, func);
    if (node == lowest) {
        lowest = this->tree.next(node);
    } else if (node == highest) {
        highest = this->tree.prev(node);
    }
    delete node;
}

libcxx::pair<int, libcxx::vector<libcxx::pair<addr_t, size_t>>>
vma::free(addr_t addr, size_t size)
{
    addr_t end    = addr + size;
    vmregion* vma = find(addr);
    libcxx::vector<libcxx::pair<addr_t, size_t>> vmas;
    if (!vma) {
        return libcxx::make_pair(0, vmas);
    }
    vmregion* prev = this->tree.prev(vma);
    if (vma->start() > end) {
        return libcxx::make_pair(0, vmas);
    }
    if (vma->start() < addr) {
        this->split(vma, addr);
        prev = vma;
    }
    vmregion* last = find(end);
    if (last && last->start() < end) {
        this->split(last, end);
    }
    vma = (prev) ? this->tree.next(prev) : this->tree.get_root();
    do {
        // Compute addresses to insert into vmas
        {
            addr_t zone_start = libcxx::max(vma->start(), addr);
            if (zone_start < vma->end()) {
                addr_t zone_end = libcxx::min(vma->end(), end);
                if (zone_end > vma->start() && zone_end != zone_start) {
                    vmas.push_back(
                        libcxx::make_pair(zone_start, zone_end - zone_start));
                }
            }
        }

        // Remove the nodes
        this->remove_node(vma);
        auto old_vma = vma;
        vma          = this->tree.next(vma);
        delete old_vma;
    } while (vma && vma->start() < end);
    if (vma) {
        this->tree.prev(vma) = prev;
    } else {
        this->highest_mapped = (prev) ? prev->end() : 0;
    }
    return libcxx::make_pair(0, vmas);
}

vmregion* vma::find(addr_t addr)
{
    vmregion* curr = tree.get_root();
    vmregion* ret  = nullptr;
    while (curr) {
        if (curr->end() > addr) {
            ret = curr;
            if (ret->start() <= addr) {
                break;
            }
            curr = this->tree.left(curr);
        } else
            curr = this->tree.right(curr);
    }
    return ret;
}

void vma::calculate_largest_subgap(vmregion* section)
{
    size_t max = 0;
    if (this->tree.prev(section)) {
        max = section->start() - this->tree.prev(section)->end();
    } else {
        max = section->start() - this->lower_bound;
    }
    if (this->tree.next(section)) {
        addr_t gap = this->tree.next(section)->start() - section->end();
        if (gap > max)
            max = gap;
    } else {
        addr_t gap = this->upper_bound - section->end();
        if (gap > max)
            max = gap;
    }
    if (this->tree.left(section)) {
        if (this->tree.left(section)->largest_subgap > max) {
            max = this->tree.left(section)->largest_subgap;
        }
    }
    if (this->tree.right(section)) {
        if (this->tree.right(section)->largest_subgap > max) {
            max = this->tree.right(section)->largest_subgap;
        }
    }
    section->largest_subgap = max;
}

libcxx::optional<addr_t> vma::allocate(addr_t hint, size_t size)
{
    auto res = locate_range(hint, size);
    if (res) {
        add_vmregion(*res, size);
    }
    return res;
}

libcxx::optional<addr_t> vma::allocate_reverse(addr_t hint, size_t size)
{
    auto res = locate_range_reverse(hint, size);
    if (res) {
        add_vmregion(*res, size);
    }
    return res;
}

void vma::reset()
{
    /*
     * WARNING: CRITICAL CODE
     * After this loop all the nodes until we reset the tree itself, the tree
     * will be in an invalid state, so do NOT call anything else.
     * TODO: We should probably take a lock
     */
    vmregion* region = lowest;
    while (region != nullptr) {
        vmregion* next = this->tree.next(region);
        delete region;
        region = next;
    }
    // Reset the rb tree
    this->tree.reset();

    this->highest_mapped = 0;
    this->lowest = this->highest = nullptr;
    // Good to go, we can use all functionality again
}
} // namespace memory
