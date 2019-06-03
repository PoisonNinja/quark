#include <arch/mm/layout.h>
#include <kernel.h>
#include <mm/page.h>
#include <mm/virtual.h>

namespace memory
{
namespace pagedb
{
namespace
{
struct page* pages = reinterpret_cast<struct page*>(PHYS_START);
addr_t highest     = 0;
} // namespace
void init(boot::info& info)
{
    highest = info.highest;
    size_t num_pages =
        (memory::virt::align_up(info.highest) / memory::virt::PAGE_SIZE) *
        sizeof(struct page);
    memory::virt::map_range(reinterpret_cast<addr_t>(pages), num_pages,
                            PAGE_WRITABLE);
}

struct page* get(addr_t address)
{
    if (address > highest) {
        kernel::panic("Attempted to get page beyond highest, %pX vs %pX\n",
                      address, highest);
    }
    addr_t real = memory::virt::align_down(address);
    return pages + (real / memory::virt::PAGE_SIZE);
}

addr_t addr(page* page)
{
    return (page - pages) * memory::virt::PAGE_SIZE;
}
} // namespace pagedb
} // namespace memory
