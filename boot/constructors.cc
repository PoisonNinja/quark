#include <boot/constructors.h>

namespace Boot
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
}  // namespace Boot
