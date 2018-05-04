#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <kernel.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
bool test(addr_t v)
{
    struct page_table* pd = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::recursive_entry);
    struct page_table* pt = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::pd_index(v));
    if (!pd->pages[Memory::X86::pd_index(v)].present ||
        !pt->pages[Memory::X86::pt_index(v)].present)
        return false;
    return true;
}
}  // namespace Virtual
}  // namespace Memory
