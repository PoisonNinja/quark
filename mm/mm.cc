#include <boot/info.h>
#include <mm/physical.h>

namespace memory
{
void __attribute__((weak)) arch_pre_init(struct boot::info& info)
{
}

void __attribute__((weak)) arch_init(struct boot::info& info)
{
}
void __attribute__((weak)) arch_post_init(struct boot::info& info)
{
}

void init(struct boot::info& info)
{
    arch_pre_init(info);
    physical::init(info);
    arch_init(info);
    physical::finalize();
    arch_post_init(info);
}
} // namespace memory
