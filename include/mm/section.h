#pragma once

#include <lib/list.h>

namespace Memory
{
class Section
{
public:
    Section(addr_t start, size_t size);
    Node<Section> node;

    bool operator==(const Section& b);
    bool operator!=(const Section& b);

    addr_t start();
    addr_t end();
    size_t size();

private:
    addr_t _start;
    size_t _size;
};

class SectionManager
{
public:
    SectionManager(addr_t s, addr_t e);
    ~SectionManager();

    bool add_section(addr_t start, size_t size, bool safe = false);
    bool locate_range(addr_t& start, addr_t hint, size_t size);

private:
    addr_t start, end;
    List<Section, &Section::node> sections;
};
}
