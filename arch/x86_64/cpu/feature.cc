#include <arch/cpu/feature.h>

namespace CPU
{
namespace X64
{
static void cpuid(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx)
{
    __asm__("cpuid"
            : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
            : "0"(*eax), "2"(*ecx)
            : "memory");
}

static uint32_t features[10];

void detect()
{
}
}
}
