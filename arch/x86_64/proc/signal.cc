#include <arch/cpu/registers.h>
#include <kernel.h>
#include <lib/string.h>
#include <proc/process.h>
#include <proc/signal.h>
#include <proc/thread.h>

struct stack_frame {
    uint64_t ret_location;
    ThreadContext ctx;
};

void Thread::setup_signal(struct ksignal* ksig,
                          struct ThreadContext* original_state,
                          struct ThreadContext* new_state)
{
    String::memcpy(new_state, original_state, sizeof(*new_state));
    addr_t stack_location;
    if (ksig->use_altstack) {
        stack_location = reinterpret_cast<addr_t>(this->signal_stack.ss_sp) +
                         this->signal_stack.ss_size;
    } else {
        stack_location = original_state->rsp;
    }
    // Construct a stack return frame
    new_state->rsp = stack_location;
    new_state->rsp -= 128;
    new_state->rsp -= sizeof(struct stack_frame);
    new_state->rsp &= ~(16UL - 1UL);
    struct stack_frame* frame = (struct stack_frame*)new_state->rsp;

    String::memcpy(&frame->ctx, original_state, sizeof(*original_state));
    frame->ret_location = this->parent->sigreturn;

    Log::printk(Log::DEBUG, "Going to return to gadget at %p\n",
                frame->ret_location);
    Log::printk(Log::DEBUG, "Frame at %p\n", frame);

    new_state->rip = (uint64_t)ksig->sa->sa_handler;
    new_state->rdi = ksig->signum;
    new_state->rsi = 0;
    new_state->rsp = (uint64_t)frame;
}