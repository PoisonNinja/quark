#pragma once

#include <proc/process.h>
#include <proc/thread.h>
#include <types.h>

namespace Scheduler
{
status_t insert(Thread* thread);
status_t remove(Thread* thread);

void init();
void tick(struct InterruptContext* ctx);

Process* get_current_process();
Thread* get_current_thread();

pid_t get_free_pid();
};
