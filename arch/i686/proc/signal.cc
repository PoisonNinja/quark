#include <arch/cpu/registers.h>
#include <kernel.h>
#include <lib/string.h>
#include <proc/process.h>
#include <proc/signal.h>
#include <proc/thread.h>

struct stack_frame {
    uint32_t ret_location;
    uint32_t signum;
    siginfo_t* siginfo_address;
    ucontext_t* ucontext_address;
    ucontext_t ucontext;
    siginfo_t siginfo;
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
        stack_location = original_state->esp;
    }
    // Construct a stack return frame
    new_state->esp = stack_location;
    new_state->esp -= sizeof(struct stack_frame);
    new_state->esp &= ~(16UL - 1UL);
    struct stack_frame* frame = (struct stack_frame*)new_state->esp;

    String::memcpy(&frame->siginfo, ksig->siginfo, sizeof(frame->siginfo));
    String::memcpy(&frame->ucontext, ksig->ucontext, sizeof(frame->ucontext));
    frame->ret_location = this->parent->sigreturn;
    frame->signum = ksig->signum;
    frame->siginfo_address = &frame->siginfo;
    frame->ucontext_address = &frame->ucontext;

    Log::printk(Log::LogLevel::DEBUG, "Going to return to gadget at %p\n",
                frame->ret_location);
    Log::printk(Log::LogLevel::DEBUG, "Frame at %p\n", frame);

    new_state->eip = (uint32_t)ksig->sa->sa_handler;
}

namespace Signal
{
void encode_mcontext(mcontext_t* mctx, struct ThreadContext* ctx)
{
    mctx->gregs[REG_EDI] = ctx->edi;
    mctx->gregs[REG_ESI] = ctx->esi;
    mctx->gregs[REG_EBP] = ctx->ebp;
    mctx->gregs[REG_ESP] = ctx->esp;
    mctx->gregs[REG_EBX] = ctx->ebx;
    mctx->gregs[REG_EDX] = ctx->edx;
    mctx->gregs[REG_ECX] = ctx->ecx;
    mctx->gregs[REG_EAX] = ctx->eax;
    mctx->gregs[REG_EIP] = ctx->eip;
    mctx->gregs[REG_EFL] = ctx->eflags;

    // // TODO: Encode GS and FS
}

void decode_mcontext(mcontext_t* mctx, struct ThreadContext* ctx)
{
    ctx->edi = mctx->gregs[REG_EDI];
    ctx->esi = mctx->gregs[REG_ESI];
    ctx->ebp = mctx->gregs[REG_EBP];
    ctx->esp = mctx->gregs[REG_ESP];
    ctx->ebx = mctx->gregs[REG_EBX];
    ctx->edx = mctx->gregs[REG_EDX];
    ctx->ecx = mctx->gregs[REG_ECX];
    ctx->eax = mctx->gregs[REG_EAX];
    ctx->eip = mctx->gregs[REG_EIP];
    ctx->eflags = mctx->gregs[REG_EFL];
    // // TODO: Decode CS, GS, and FS
}
}  // namespace Signal