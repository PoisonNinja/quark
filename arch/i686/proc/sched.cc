#include <proc/sched.h>

namespace Scheduler
{
void __attribute__((noreturn)) yield()
{
    __asm__ __volatile__("int $0x81");
    __builtin_unreachable();
}
}  // namespace Scheduler
