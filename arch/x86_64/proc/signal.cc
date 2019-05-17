#include <arch/cpu/registers.h>
#include <kernel.h>
#include <lib/string.h>
#include <proc/process.h>
#include <proc/signal.h>
#include <proc/thread.h>

struct stack_frame {
    uint64_t ret_location;
#ifndef X86_64
    uint32_t signum;
    siginfo_t* siginfo_address;
    ucontext_t* ucontext_address;
#endif
    ucontext_t ucontext;
    siginfo_t siginfo;
};

void thread::setup_signal(struct ksignal* ksig,
                          struct thread_context* original_state,
                          struct thread_context* new_state)
{
    libcxx::memcpy(new_state, original_state, sizeof(*new_state));
    addr_t stack_location;
    if (ksig->use_altstack) {
        stack_location = reinterpret_cast<addr_t>(this->signal_stack.ss_sp) +
                         this->signal_stack.ss_size;
    } else {
#ifdef X86_64
        stack_location = original_state->rsp;
#else
        stack_location = original_state->esp;
#endif
    }
    // Construct a stack return frame
#ifdef X86_64
    new_state->rsp = stack_location;
    new_state->rsp -= 128;
    new_state->rsp -= sizeof(struct stack_frame);
    new_state->rsp &= ~(16UL - 1UL);
    struct stack_frame* frame = (struct stack_frame*)new_state->rsp;
#else
    new_state->esp = stack_location;
    new_state->esp -= sizeof(struct stack_frame);
    new_state->esp &= ~(16UL - 1UL);
    struct stack_frame* frame = (struct stack_frame*)new_state->esp;
#endif

    libcxx::memcpy(&frame->siginfo, ksig->siginfo, sizeof(frame->siginfo));
    libcxx::memcpy(&frame->ucontext, ksig->ucontext, sizeof(frame->ucontext));
    frame->ret_location = this->parent->sigreturn;
#ifndef X86_64
    frame->signum           = ksig->signum;
    frame->siginfo_address  = &frame->siginfo;
    frame->ucontext_address = &frame->ucontext;
#endif

    log::printk(log::log_level::DEBUG, "Going to return to gadget at %p\n",
                frame->ret_location);
    log::printk(log::log_level::DEBUG, "Frame at %p\n", frame);

#ifdef X86_64
    new_state->rip = (uint64_t)ksig->sa->sa_handler;
    new_state->rdi = ksig->signum;

    /*
     * Technically SA_SIGINFO specifies this, but programmers make mistakes. To
     * prevent them from getting segmentation faults we still pass these in
     * as arguments regardless of whether SA_SIGINFO is used. Not POSIX
     * compliant, but meh
     */
    new_state->rsi = (uint64_t)&frame->siginfo;
    new_state->rdx = (uint64_t)&frame->ucontext;
    new_state->rsp = (uint64_t)frame;
#else
    new_state->eip = (uint32_t)ksig->sa->sa_handler;
#endif
}

namespace signal
{
void encode_mcontext(mcontext_t* mctx, struct thread_context* ctx)
{
#ifdef X86_64
    mctx->gregs[REG_R8]  = ctx->r8;
    mctx->gregs[REG_R9]  = ctx->r9;
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
#else
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
#endif
}

void decode_mcontext(mcontext_t* mctx, struct thread_context* ctx)
{
#ifdef X86_64
    ctx->r8     = mctx->gregs[REG_R8];
    ctx->r9     = mctx->gregs[REG_R9];
    ctx->r10    = mctx->gregs[REG_R10];
    ctx->r11    = mctx->gregs[REG_R11];
    ctx->r12    = mctx->gregs[REG_R12];
    ctx->r13    = mctx->gregs[REG_R13];
    ctx->r14    = mctx->gregs[REG_R14];
    ctx->r15    = mctx->gregs[REG_R15];
    ctx->rdi    = mctx->gregs[REG_RDI];
    ctx->rsi    = mctx->gregs[REG_RSI];
    ctx->rbp    = mctx->gregs[REG_RBP];
    ctx->rbx    = mctx->gregs[REG_RBX];
    ctx->rdx    = mctx->gregs[REG_RDX];
    ctx->rax    = mctx->gregs[REG_RAX];
    ctx->rcx    = mctx->gregs[REG_RCX];
    ctx->rsp    = mctx->gregs[REG_RSP];
    ctx->rip    = mctx->gregs[REG_RIP];
    ctx->rflags = mctx->gregs[REG_EFL];
// TODO: Decode CS, GS, and FS
#else
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
#endif
}
} // namespace signal
