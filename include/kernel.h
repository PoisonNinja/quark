#pragma once

#include <kernel/log.h>

namespace Kernel
{
void __attribute__((noreturn)) panic(const char* message, ...);
}
