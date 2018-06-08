#include <arch/cpu/registers.h>
#include <kernel.h>
#include <lib/string.h>
#include <proc/process.h>
#include <proc/signal.h>
#include <proc/thread.h>

struct stack_frame {
    uint64_t ret_location;
    ucontext_t ucontext;
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

    String::memcpy(&frame->ucontext, ksig->ucontext, sizeof(frame->ucontext));
    frame->ret_location = this->parent->sigreturn;

    Log::printk(Log::DEBUG, "Going to return to gadget at %p\n",
                frame->ret_location);
    Log::printk(Log::DEBUG, "Frame at %p\n", frame);

    new_state->rip = (uint64_t)ksig->sa->sa_handler;
    new_state->rdi = ksig->signum;
    new_state->rsi = 0;
    new_state->rdx = (uint64_t)&frame->ucontext;
    new_state->rsp = (uint64_t)frame;
}

namespace Signal
{
void encode_mcontext(mcontext_t* mctx, struct ThreadContext* ctx)
{
    mctx->gregs[REG_R8] = ctx->r8;
    mctx->gregs[REG_R9] = ctx->r9;
    mctx->gregs[REG_R10] = ctx->r10;
    mctx->gregs[REG_R11] = ctx->r11;
    mctx->gregs[REG_R12] = ctx->r12;
    mctx->gregs[REG_R13] = ctx->r13;
    mctx->gregs[REG_R14] = ctx->r14;
    mctx->gregs[REG_R15] = ctx->r15;
    mctx->gregs[REG_RDI] = ctx->rdi;
    mctx->gregs[REG_RSI] = ctx->rsi;
    mctx->gregs[REG_RBP] = ctx->rbp;
    mctx->gregs[REG_RBX] = ctx->rbx;
    mctx->gregs[REG_RDX] = ctx->rdx;
    mctx->gregs[REG_RAX] = ctx->rax;
    mctx->gregs[REG_RCX] = ctx->rcx;
    mctx->gregs[REG_RSP] = ctx->rsp;
    mctx->gregs[REG_RIP] = ctx->rip;
    mctx->gregs[REG_EFL] = ctx->rflags;
    // TODO: Encode GS and FS
    mctx->gregs[REG_CSGSFS] = ctx->cs;
}

void decode_mcontext(mcontext_t* mctx, struct ThreadContext* ctx)
{
    ctx->r8 = mctx->gregs[REG_R8];
    ctx->r9 = mctx->gregs[REG_R9];
    ctx->r10 = mctx->gregs[REG_R10];
    ctx->r11 = mctx->gregs[REG_R11];
    ctx->r12 = mctx->gregs[REG_R12];
    ctx->r13 = mctx->gregs[REG_R13];
    ctx->r14 = mctx->gregs[REG_R14];
    ctx->r15 = mctx->gregs[REG_R15];
    ctx->rdi = mctx->gregs[REG_RDI];
    ctx->rsi = mctx->gregs[REG_RSI];
    ctx->rbp = mctx->gregs[REG_RBP];
    ctx->rbx = mctx->gregs[REG_RBX];
    ctx->rdx = mctx->gregs[REG_RDX];
    ctx->rax = mctx->gregs[REG_RAX];
    ctx->rcx = mctx->gregs[REG_RCX];
    ctx->rsp = mctx->gregs[REG_RSP];
    ctx->rip = mctx->gregs[REG_RIP];
    ctx->rflags = mctx->gregs[REG_EFL];
    // TODO: Decode CS, GS, and FS
}
}  // namespace Signal