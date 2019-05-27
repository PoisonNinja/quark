#include <proc/sched.h>

namespace scheduler
{
void yield()
{
    __asm__ __volatile__("int $0x81");
}
} // namespace scheduler
