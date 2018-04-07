#include <cpu/interrupt.h>
#include <kernel.h>
#include <lib/list.h>
#include <lib/string.h>
#include <mm/virtual.h>
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
        // TODO: Get rid of hlt
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

void tick(struct InterruptContext* ctx)
{
    if (current_thread) {
        current_thread->save_state(ctx);
    }
    if (runnable.empty()) {
        kidle->load_state(ctx);
        current_thread = kidle;
        return;
    }
    Thread& next = runnable.front();
    remove(&next);
    runnable.push_back(next);
    if (current_thread) {
        if (current_thread->parent->address_space !=
            next.parent->address_space) {
            Memory::Virtual::set_address_space_root(next.parent->address_space);
        }
    } else {
        Memory::Virtual::set_address_space_root(next.parent->address_space);
    }
    next.load_state(ctx);
    current_thread = &next;
}

extern "C" void load_registers(InterruptContext* ctx);

void artifial_tick()
{
    struct InterruptContext ctx;
    String::memset(&ctx, 0, sizeof(struct InterruptContext));
    tick(&ctx);
    load_registers(&ctx);
}

void init()
{
    Log::printk(Log::INFO, "Initializing scheduler...\n");
    kernel_process = new Process();
    kernel_process->address_space = Memory::Virtual::get_address_space_root();
    // TODO: Move this to architecture specific
    Thread* kinit = new Thread(kernel_process);
    kidle = new Thread(kernel_process);
    String::memset(&kidle->cpu_ctx, 0, sizeof(kidle->cpu_ctx));
    kidle->cpu_ctx.ds = kidle->cpu_ctx.ss = 0x10;
    kidle->cpu_ctx.cs = 0x08;
    kidle->cpu_ctx.rip = reinterpret_cast<addr_t>(idle);
    kidle->cpu_ctx.rsp = kidle->cpu_ctx.rbp =
        reinterpret_cast<addr_t>(new uint8_t[0x1000]) + 0x1000;
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

Process* get_current_process()
{
    return current_thread->parent;
}

Thread* get_current_thread()
{
    return current_thread;
}
}
