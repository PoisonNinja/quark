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

static pid_t next_pid = 0;

pid_t get_free_pid()
{
    // TODO: Get next __FREE__ slot, not just next slot
    return next_pid++;
}

bool insert(Thread* thread)
{
    runnable.push_front(*thread);
    return true;
}

bool remove(Thread* thread)
{
    for (auto it = runnable.begin(); it != runnable.end(); ++it) {
        auto& value = *it;
        if (&value == thread) {
            runnable.erase(it);
            return true;
        }
    }
    return false;
}

Thread* next()
{
    Thread* next = nullptr;
    if (runnable.empty()) {
        next = kidle;
    } else {
        next = &runnable.front();
        remove(next);
        runnable.push_back(*next);
    }
    return next;
}

void switch_context(struct InterruptContext* ctx, Thread* current, Thread* next)
{
    save_context(ctx, &current->cpu_ctx);
    set_stack(next->kernel_stack);
    if (current) {
        if (current->parent->address_space != next->parent->address_space) {
            Memory::Virtual::set_address_space_root(
                next->parent->address_space);
        }
    } else {
        Memory::Virtual::set_address_space_root(next->parent->address_space);
    }
    load_context(ctx, &next->cpu_ctx);
}

void switch_next(struct InterruptContext* ctx)
{
    Thread* next_thread = next();
    switch_context(ctx, current_thread, next_thread);
    current_thread = next_thread;
}

void __attribute__((noreturn)) yield()
{
    __asm__ __volatile__("int $0x81");
    // Shut up GCC
    for (;;)
        __asm__("hlt");
}

void yield_switch(int, void*, struct InterruptContext* ctx)
{
    Thread* next_thread = next();
    switch_context(ctx, current_thread, next_thread);
    current_thread = next_thread;
}

Interrupt::Handler yield_handler(yield_switch, "yield", &yield_handler);

void init()
{
    Log::printk(Log::INFO, "Initializing scheduler...\n");
    Interrupt::register_handler(0x81, yield_handler);
    kernel_process = new Process(nullptr);
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
