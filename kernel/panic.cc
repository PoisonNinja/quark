#include <cpu/cpu.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/stacktrace.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <stdarg.h>

namespace Kernel
{
constexpr size_t panic_max = 1024;

static char panic_buffer[Kernel::panic_max];

void __attribute__((noreturn)) panic(const char* format, ...)
{
    Interrupt::disable();
    libcxx::memset(panic_buffer, 0, Kernel::panic_max);
    va_list args;
    va_start(args, format);
    libcxx::vsnprintf(panic_buffer, Kernel::panic_max, format, args);
    va_end(args);
    Log::printk(Log::LogLevel::ERROR, "%s", panic_buffer);
    do_stack_trace();
    for (;;)
        CPU::halt();
}
} // namespace Kernel
