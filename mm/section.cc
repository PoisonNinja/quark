#include <kernel.h>
#include <lib/algorithm.h>
#include <lib/rb.h>
#include <mm/section.h>
#include <mm/virtual.h>

namespace Memory
{
Section::Section(addr_t start, size_t size)
    : prev(nullptr)
    , next(nullptr)
    , largest_subgap(0)
    , _start(start)
    , _size(size)
{
}

Section::Section(Section& other)
    : Section(other._start, other._size)
{
}

addr_t Section::start() const
{
    return _start;
}

addr_t Section::end() const
{
    return _start + _size;
}

size_t Section::size() const
{
    return _size;
}

bool Section::operator==(const Section& b)
{
    return (_start == b._start) && (_size == b._size);
}

bool Section::operator!=(const Section& b)
{
    return !(*this == b);
}

bool Section::operator<(const Section& b)
{
    return this->_start < b._start;
}

SectionManager::SectionManager(addr_t start, addr_t end)
    : start(start)
    , end(end)
    , highest(0)
{
}

SectionManager::SectionManager(SectionManager& other)
    : SectionManager(other.start, other.end)
{
}

SectionManager::~SectionManager()
{
}

bool SectionManager::add_section(addr_t start, size_t size)
{
    start = Memory::Virtual::align_down(start);
    size  = Memory::Virtual::align_up(size);
    Log::printk(Log::LogLevel::INFO, "add_section: %p, %zX\n", start, size);
    Section* section = new Section(start, size);
    // TODO: Sanity checks for overlaps, exceeding bounds
    // Of course, that probably requires support from rbtree
    section->prev = this->calculate_prev(start);
    if (section->prev) {
        Section* tnext = const_cast<Section*>(section->prev->next);
        Section* tprev = const_cast<Section*>(section->prev);
        tprev->next    = section;
        section->next  = tnext;
    } else {
        if (this->sections.parent(const_cast<const Section*>(section))) {
            section->next = const_cast<const Section*>(section);
        } else {
            section->next = nullptr;
        }
    }
    if (section->next) {
        Section* tnext = const_cast<Section*>(section->next);
        tnext->prev    = section;
    }
    auto func = libcxx::bind(&SectionManager::calculate_largest_subgap, this,
                             libcxx::placeholders::_1);
    if (section->end() > highest) {
        highest = section->end();
    }
    this->sections.insert(*section, func);
    this->print();
    return true;
}

bool SectionManager::locate_range(addr_t& ret, addr_t hint, size_t size)
{
    const Section* curr = sections.get_root();
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

const Section* SectionManager::calculate_prev(addr_t addr)
{
    const Section* curr = sections.get_root();
    const Section* prev = nullptr;
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

void SectionManager::calculate_largest_subgap(Section* section)
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
    if (this->sections.left(const_cast<const Section*>(section))) {
        if (this->sections.left(const_cast<const Section*>(section))
                ->largest_subgap > max) {
            max = this->sections.left(const_cast<const Section*>(section))
                      ->largest_subgap;
        }
    }
    if (this->sections.right(const_cast<const Section*>(section))) {
        if (this->sections.right(const_cast<const Section*>(section))
                ->largest_subgap > max) {
            max = this->sections.right(const_cast<const Section*>(section))
                      ->largest_subgap;
        }
    }
    section->largest_subgap = max;
}

void SectionManager::reset()
{
}

void SectionManager::print2DUtil(const Section* root, int space)
{
    // Base case
    if (root == NULL)
        return;

    // Increase distance between levels
    space += 10;

    // Process right child first
    print2DUtil(root->node.right, space);

    // Print current node after space
    // count
    Log::printk(Log::LogLevel::CONTINUE, "\n");
    for (int i = 10; i < space; i++)
        Log::printk(Log::LogLevel::CONTINUE, " ");
    Log::printk(Log::LogLevel::CONTINUE, "%p - %p (%zX)\n", root->start(),
                root->end(), root->largest_subgap);

    // Process left child
    print2DUtil(root->node.left, space);
}

// Wrapper over print2DUtil()
void SectionManager::print()
{
    Log::printk(Log::LogLevel::INFO, "=======================\n");
    // Pass initial space count as 0
    print2DUtil(this->sections.get_root(), 0);
    Log::printk(Log::LogLevel::INFO, "=======================\n");
}
} // namespace Memory
