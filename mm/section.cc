#include <kernel.h>
#include <mm/section.h>
#include <mm/virtual.h>

namespace Memory
{
Section::Section(addr_t start, size_t size)
{
    _start = start;
    _size = size;
}

addr_t Section::start()
{
    return _start;
}

addr_t Section::end()
{
    return _start + _size;
}

size_t Section::size()
{
    return _size;
}

SectionManager::SectionManager(addr_t s, addr_t e)
{
    Section* section = new Section(s, e - s);
    sections.push_front(*section);
}

SectionManager::~SectionManager()
{
    for (auto& i : sections) {
        delete &i;
    }
}

bool SectionManager::add_section(addr_t start, size_t size)
{
    start = Memory::Virtual::align_down(start);
    size = Memory::Virtual::align_up(size);
    size_t end = start + size;
    for (auto it = sections.begin(); it != sections.end(); it++) {
        auto& i = *it;
        if (i.start() >= start && start + size <= i.end()) {
            addr_t orig = i.start();
            addr_t orig_end = i.end();
            // Iterator invalidated, don't use anymore
            sections.erase(it);
            /*
             * Scenarios (| = boundary, * = free, x = allocated):
             * |**xxxx***|, |xxxxxxxxxx|, |xxxx*****|, |****xxxxxx|
             */
            if (start > orig && end < orig_end) {
                Section* a = new Section(orig, start - orig);
                Section* b = new Section(end, orig_end - end);
                sections.push_back(*a);
                sections.push_back(*b);
            } else if (start == orig && end == orig_end) {
                // Nothing, since we already deleted it
            } else if (start == orig && end < orig_end) {
                Section* a = new Section(end, orig_end - end);
                sections.push_back(*a);
            } else if (start > orig && end == orig_end) {
                Section* a = new Section(orig, start - orig);
                sections.push_back(*a);
            }
            return true;
        }
    }
    return false;
}

bool SectionManager::locate_range(addr_t& start, addr_t hint, size_t size)
{
    bool found = false;
    size_t best = 0;
    for (auto& i : sections) {
        if (i.start() <= hint && i.end() >= hint + size) {
            start = i.start();
            return true;
        }
        size_t prelim = 0;
        if (i.size() >= size) {
            prelim = (i.start() < hint) ? hint - i.start() : i.start() - hint;
            if (!found) {
                start = i.start();
                best = prelim;
                found = true;
            } else {
                if (prelim < best) {
                    best = prelim;
                    start = i.start();
                }
            }
        }
    }
    return found;
}
}
