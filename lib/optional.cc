#include <kernel.h>

void __throw_bad_optional_access(const char* s)
{
    kernel::panic("%s\n", s);
}
