#pragma once

#include <proc/process.h>
#include <proc/thread.h>
#include <types.h>

namespace Scheduler
{
bool insert(Thread* thread);
bool remove(Thread* thread);

void init();
void switch_next(struct InterruptContext* ctx);
void yield();

Process* get_current_process();
Thread* get_current_thread();

pid_t get_free_pid();
};
