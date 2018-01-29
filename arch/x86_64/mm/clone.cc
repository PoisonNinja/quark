#include <arch/mm/mm.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
status_t arch_clone()
{
    addr_t clone = Memory::Physical::get();
    struct page_table* pml4 =
        reinterpret_cast<struct page_table*>(Memory::X64::read_cr3());
    addr_t original_pml4 = pml4->pages[RECURSIVE_ENTRY].address;
    pml4->pages[RECURSIVE_ENTRY].address = clone / 0x1000;
    Memory::X64::write_cr3(reinterpret_cast<addr_t>(pml4));

    struct page page;

    for (int i = 0; i < 512; i++) {
        Log::printk(Log::DEBUG, "%d: %p\n", i,
                    Memory::X64::decode_fractal(i, 0, 0, 0));
        // Memory::Virtual::get(
        //     reinterpret_cast<addr_t>(Memory::X64::decode_fractal(i, 0, 0,
        //     0)), page);
        // if (i >= 256 && page.present) {
        //     Log::printk(Log::DEBUG, "In kernel space, linking page
        //     tables\n");
        // }
    }

    pml4->pages[RECURSIVE_ENTRY].address = original_pml4;
    Memory::X64::write_cr3(reinterpret_cast<addr_t>(pml4));
    return SUCCESS;
}
}
}