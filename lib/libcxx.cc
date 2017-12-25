#include <kernel.h>
#include <lib/libcxx.h>

namespace libcxx
{
typedef void (*constructor_t)();

void constructors_initialize(void* __constructors_start,
                             void* __constructors_end)
{
    constructor_t* start = (constructor_t*)__constructors_start;
    constructor_t* end = (constructor_t*)__constructors_end;

    for (constructor_t* current = start; current != end; ++current) {
        (*current)();
    }
}

extern "C" __attribute__((noreturn)) void __cxa_pure_virtual()
{
    Kernel::panic("__cxa_pure_virtual encountered!\n");
}
}
