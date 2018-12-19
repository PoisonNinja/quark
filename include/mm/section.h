#pragma once

#include <lib/rb.h>

namespace Memory
{
class Section
{
public:
    Section(addr_t start, size_t size);
    Section(Section& other);

    libcxx::rbnode<Section> node;

    bool operator==(const Section& b);
    bool operator!=(const Section& b);
    bool operator<(const Section& b);

    addr_t start() const;
    addr_t end() const;
    size_t size() const;

    // Additional metadata to speed up accesses
    const Section *prev, *next;
    size_t largest_subgap; // Same idea as Linux

private:
    addr_t _start;
    size_t _size;
};

class SectionManager
{
public:
    SectionManager(addr_t s, addr_t e);
    SectionManager(SectionManager& other);
    ~SectionManager();

    void print2DUtil(const Section* root, int space);
    void print();

    bool add_section(addr_t start, size_t size);
    bool locate_range(addr_t& start, addr_t hint, size_t size);

    void reset();

private:
    addr_t start, end;
    addr_t highest;

    void calculate_largest_subgap(Section* section);
    const Section* calculate_prev(addr_t addr);

    libcxx::rbtree<Section, &Section::node> sections;
};
} // namespace Memory
