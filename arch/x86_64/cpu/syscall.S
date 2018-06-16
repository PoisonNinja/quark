# %include "common.inc"

.extern syscall_trampoline

.global syscall_sysret_wrapper
syscall_sysret_wrapper:
    swapgs              # Thread struct is saved in KernelGSBase, swap it into GS
    mov %rsp, %gs:80    # Save user RSP into thread->tcontext.rsp
    mov %gs:184, %rsp   # Load kernel RSP from thread->kernel_stack.

    # Build a simulated interrupt frame
    pushq $0x1B   # SS
    pushq %gs:80 # User RSP
    pushq %r11   # RFLAGS
    pushq $0x23  # CS
    pushq %rcx   # RIP

    pushq $0     # Error code
    pushq $0     # Interrupt number

    push %rax     # save current rax
    push %rbx     # save current rbx
    push %rcx     # save current rcx
    push %rdx     # save current rdx
    push %rbp     # save current rbp
    push %rdi     # save current rdi
    push %rsi     # save current rsi
    push %r8      # save current r8
    push %r9      # save current r9
    push %r10     # save current r10
    push %r11     # save current r11
    push %r12     # save current r12
    push %r13     # save current r13
    push %r14     # save current r14
    push %r15     # save current r15

    pushq $0x1B  # DS

    # Store FS
    mov $0xC0000100, %rcx
    rdmsr
    shl $32, %rdx
    or %rax, %rdx
    push %rdx

    # Store GS
    mov $0xC0000101, %rcx
    rdmsr
    shl $32, %rdx
    or %rax, %rdx
    push %rdx

    swapgs              # Swap back GS values. This is important because
                        # we may not reach the end of this function, especially
                        # \if the system call is sys_exit. If that happens,
                        # future syscalls will panic because GS contains
                        # the wrong value

    mov %rsp, %rdi
    call syscall_trampoline

    # There isn't really a reason for system calls to be modifying FS and GS
    pop %r15
    pop %r15

    pop %r15    # DS

    pop %r15     # restore current r15
    pop %r14     # restore current r14
    pop %r13     # restore current r13
    pop %r12     # restore current r12
    pop %r11     # restore current r11
    pop %r10     # restore current r10
    pop %r9      # restore current r9
    pop %r8      # restore current r8
    pop %rsi     # restore current rsi
    pop %rdi     # restore current rdi
    pop %rbp     # restore current rbp
    pop %rdx     # restore current rdx
    pop %rcx     # restore current rcx
    pop %rbx     # restore current rbx
    pop %rax     # restore current rax

    add $16, %rsp  # Skip the rest of the IRET frame
    pop %rcx       # RIP
    add $8, %rsp
    pop %r11       # RFLAGS
    pop %rsp

    sysretq        # Return
