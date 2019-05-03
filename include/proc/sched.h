#pragma once

#include <proc/process.h>
#include <proc/thread.h>
#include <types.h>

namespace scheduler
{
void idle();

bool insert(thread* thread);
bool remove(thread* thread);

void init();
void late_init();
void switch_next(struct interrupt_context* ctx);
void yield();

process* get_current_process();
thread* get_current_thread();

pid_t get_free_pid();

bool online();

bool add_process(process* process);
process* find_process(pid_t pid);
bool remove_process(pid_t pid);
}; // namespace scheduler
