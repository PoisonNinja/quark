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

Section::Section(Section& other)
{
    _start = other._start;
    _size = other._size;
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

bool Section::operator==(const Section& b)
{
    return (_start == b._start) && (_size == b._size);
}

bool Section::operator!=(const Section& b)
{
    return !(*this == b);
}

SectionManager::SectionManager(addr_t s, addr_t e)
{
    start = s;
    _end = e;
}

SectionManager::SectionManager(SectionManager& other)
{
    start = other.start;
    _end = other._end;
    for (auto& section : other.sections) {
        Section* s = new Section(section);
        sections.push_back(*s);
    }
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
    // Allocate a new section
    Section* section = new Section(start, size);
    // Use a dumb insertion algorithm
    // Check if the list of sections is empty
    if (sections.empty()) {
        // Since it is, we can just simply push it back
        sections.push_back(*section);
        return true;
    } else {
        /*
         * Check if the first section is above our new section
         */
        if (sections.front().start() >= section->end()) {
            // Insert into the front
            sections.push_front(*section);
            return true;
        }
        /*
         * Iterate through the sections, looking for the correct insertion
         * place. We need to use iterators instead of range based because
         * insert utilizes iterators
         */
        for (auto it = sections.begin(); it != sections.end(); it++) {
            auto i = *it;
            /*
             * Check if the section is below our new section
             */
            if (i.end() <= section->start()) {
                /*
                 * Check if this section is the last one. If it isn't, we need
                 * to consider the next section as well.
                 */
                if (it != --sections.end()) {
                    /*
                     * It isn't, so let's obtain the next section. We need to
                     * duplicate it because we can't peek at the next section
                     * without incrementing the iterator
                     */
                    auto t = it;
                    t++;
                    auto j = *t;
                    // Check if our new section is above the next section.
                    if (section->start() >= j.start()) {
                        /*
                         * It is, there are sections still before our new one.
                         * Restart the loop
                         */
                        continue;
                    }
                }
                /*
                 * We found the highest section before our new one, insert it
                 * after
                 */
                sections.insert(++it, *section);
                return true;
            }
        }
    }
    // I guess we ran out of address space
    return false;
}

bool SectionManager::locate_range(addr_t& ret, addr_t hint, size_t size)
{
    bool found = false;
    size_t best = 0;
    // Check if there are any sections already allocated
    if (sections.empty()) {
        // No, so the hint is what's valid
        ret = hint;
        return true;
    }
    // Check for gaps before the first section that can hold
    if (sections.front().start() - start > size) {
        // Check if the hint is completely inside the gap
        if (sections.front().start() >= hint + size) {
            // Yes, so hint is good
            ret = hint;
            return true;
        }
        addr_t target = sections.front().start() - size;
        // No, so set fuzz value
        ret = target;
        best = (target < hint) ? hint - target : target - hint;
        found = true;
    }
    // Iterate through all the sections
    for (auto it = sections.begin(); it != sections.end(); it++) {
        addr_t zone_start, zone_end;
        auto& i = *it;
        // Check if this section is the last one
        if (it != --sections.end()) {
            /*
             * The gap is between the end of this section and the start of
             * the next one
             */
            auto tmp = it;
            tmp++;
            auto& j = *tmp;
            zone_start = i.end();
            zone_end = j.start();
        } else {
            /*
             * Last section, so gap is between end of this section and the
             * end of the entire zone
             */
            zone_start = i.end();
            zone_end = _end;
        }
        // Calculate the zone size
        size_t zone_size = zone_end - zone_start;
        // Check for null zones (e.g. if start == start of first segment)
        if (!zone_size)
            continue;
        // Calculate the prelimary distance score
        size_t prelim = 0;
        // Check if this gap is even eligible to be scored (big enough)
        if (zone_size >= size) {
            // Check if the hint can be directly used in this zone
            if (zone_start <= hint && zone_end >= hint + size) {
                ret = hint;
                return true;
            }
            // Calculate the distance from the zone to the hint
            addr_t target;
            if (zone_start >= hint) {
                target = zone_start;
            } else {
                target = zone_end - size;
            }
            prelim = (target < hint) ? hint - target : target - hint;
            if (!found) {
                ret = target;
                best = prelim;
                found = true;
            } else {
                if (prelim < best) {
                    ret = target;
                    best = prelim;
                }
            }
        }
    }
    return found;
}

iterator<Section, &Section::node> SectionManager::begin()
{
    return sections.begin();
}

iterator<Section, &Section::node> SectionManager::end()
{
    return sections.end();
}
}
