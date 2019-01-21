#pragma once

#include <kernel/log.h>

namespace kernel
{
void __attribute__((noreturn)) panic(const char* message, ...);
}
