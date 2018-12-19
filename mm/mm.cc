#include <boot/info.h>
#include <mm/physical.h>

namespace Memory
{
extern void arch_init(struct Boot::info& info);

void init(struct Boot::info& info)
{
    Physical::init(info);
    arch_init(info);
    Physical::finalize();
}
} // namespace Memory
