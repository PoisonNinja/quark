#include <kernel.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <stdarg.h>

#define PANIC_MAX 1024

static char panic_buffer[PANIC_MAX];

namespace Kernel
{
void __attribute__((noreturn)) panic(const char* format, ...)
{
    size_t r = 0;
    String::memset(panic_buffer, 0, PANIC_MAX);
    va_list args;
    va_start(args, format);
    r = vsnprintf(panic_buffer, PANIC_MAX, format, args);
    va_end(args);
    Log::printk(Log::ERROR, "%s", panic_buffer);
    for (;;)
        asm("hlt");
}
}  // namespace Kernel
