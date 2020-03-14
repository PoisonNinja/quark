#pragma once

#include <lib/functional.h>

namespace scheduler
{
namespace work
{
using work_handler_t = libcxx::function<void(), 32>;

void schedule(work_handler_t handler);

void init();
} // namespace work
} // namespace scheduler
