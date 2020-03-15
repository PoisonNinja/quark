#include <kernel.h>

void __assert(const char* message, const char* file, int line)
{
    kernel::panic("Assertion failure: `%s` in %s:%d failed\n", message, file,
                  line);
}
