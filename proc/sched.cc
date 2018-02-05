#include <cpu/interrupt.h>
#include <kernel.h>
#include <lib/list.h>
#include <proc/sched.h>

namespace Scheduler
{
static class List<Thread, &Thread::scheduler_node> runnable;
static Thread* current_thread;
static Process* kernel_process;
static Thread* kidle;

static void idle()
{
    while (1) {
        Log::printk(Log::INFO, "Idle\n");
        __asm__("hlt");
    }
}

static pid_t next = 0;

pid_t get_free_pid()
{
    // TODO: Get next __FREE__ slot, not just next slot
    return next++;
}

status_t insert(Thread* thread)
{
    runnable.push_front(*thread);
    return SUCCESS;
}

status_t remove(Thread* thread)
{
    for (auto it = runnable.begin(); it != runnable.end(); ++it) {
        auto& value = *it;
        if (&value == thread) {
            runnable.erase(it);
            return SUCCESS;
        }
    }
    return FAILURE;
}

void tick(struct interrupt_ctx* ctx)
{
    if (runnable.empty()) {
        // load_context(kidle, ctx);
        return;
    }
    Thread& next = runnable.front();
    remove(&next);
    if (current_thread) {
        save_context(current_thread, ctx);
        runnable.push_back(*current_thread);
    }
    load_context(&next, ctx);
}

void init()
{
    Log::printk(Log::INFO, "Initializing scheduler...\n");
    // We are the root :)
    kernel_process = new Process(nullptr);
    Thread* kinit = new Thread(kernel_process);
    kidle = new Thread(kernel_process);
    kidle->cpu_ctx.rip = reinterpret_cast<addr_t>(idle);
    Scheduler::insert(kinit);
    /*
     * Set kinit as current_thread, so on the first task switch caused by the
     * timer, it will save the kernel context into the kinit task, but since
     * it's also the ONLY task right now, we will resume kinit, so in essence
     * the kernel just became a thread.
     */
    current_thread = kinit;
    Log::printk(Log::INFO, "Scheduler initialized\n");
}
}
