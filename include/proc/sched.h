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
void switch_next();

// Sets the process status, remove from the scheduler, and switch out
void sleep(thread_state state);

process* get_current_process();
thread* get_current_thread();

pid_t get_free_pid();

bool online();

bool add_process(process* process);
process* find_process(pid_t pid);
bool remove_process(pid_t pid);

thread* create_kernel_thread(void (*entry_point)(void*), void* data);
}; // namespace scheduler
