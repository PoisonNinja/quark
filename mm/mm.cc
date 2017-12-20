#include <boot/info.h>

namespace Memory
{
extern void arch_init(struct Boot::info& info);

void init(struct Boot::info& info)
{
    arch_init(info);
}
}  // namespace Memory
