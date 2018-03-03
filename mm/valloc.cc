#include <arch/mm/mm.h>
#include <kernel.h>
#include <mm/valloc.h>

namespace Memory
{
namespace Valloc
{
static addr_t valloc_current = 0xFFFFFC0000000000;

/*
 * Keep track of how many free requests we receive. If this becomes large
 * enough, we should implement free for real
 */
static size_t valloc_free_requests = 0;

addr_t allocate(size_t size)
{
    size = Memory::Virtual::align_up(size);
    addr_t current = valloc_current;
    valloc_current += size;
    return current;
}

void free(addr_t address)
{
    valloc_free_requests++;
    Log::printk(
        Log::WARNING,
        "valloc memory free was requested, but this "
        "function is not implemented! Request #%llu wanted to free %p\n",
        valloc_free_requests, address);
}
}
}