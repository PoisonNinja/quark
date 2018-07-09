#include <arch/mm/layout.h>
#include <kernel.h>
#include <mm/valloc.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Valloc
{
static addr_t valloc_current = VALLOC_START;

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
    Log::printk(Log::LogLevel::WARNING,
                "valloc memory free was requested, but this "
                "function is not implemented! Request #%zu wanted to free %p\n",
                valloc_free_requests, address);
}
}  // namespace Valloc
}  // namespace Memory
