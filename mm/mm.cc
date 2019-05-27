#include <boot/info.h>
#include <mm/physical.h>

namespace memory
{
extern void arch_init(struct boot::info& info);

void init(struct boot::info& info)
{
    arch_init(info);
    physical::init(info);
    physical::finalize();
}
} // namespace memory
