#include <proc/sched.h>

namespace Scheduler
{
void yield()
{
    __asm__ __volatile__("int $0x81");
}
}  // namespace Scheduler
