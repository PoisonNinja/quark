
extern syscall_trampoline

global syscall_sysret_wrapper
syscall_sysret_wrapper:
    swapgs              ; Thread struct is saved in KernelGSBase, swap it into GS
    mov [gs:104], rsp    ; Save user RSP into thread->cpu_ctx.rsp
    mov rsp, [gs:192]   ; Load kernel RSP from thread->kernel_stack.

    ; Build a simulated interrupt frame
    push qword 0x1B ; SS
    push qword [gs:104] ; User RSP
    push r11  ; RFLAGS
    push qword 0x23 ; CS
    push rcx  ; RIP

    push qword 0 ; Error code
    push qword 0 ; Interrupt number

    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    push qword 0x1B ; DS

    swapgs              ; Swap back GS values. This is important because
                        ; we may not reach the end of this function, especially
                        ; if the system call is sys_exit. If that happens,
                        ; future syscalls will panic because GS contains
                        ; the wrong value

    mov rdi, rsp
    call syscall_trampoline

    pop r15 ; DS

    pop r15
    pop r14
    pop r13
    pop r12
    pop rax ; Skip R11 (RFLAGS)
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rax ; Skip RCX (Return RIP)
    pop rbx
    pop rax

    add rsp, 16 ; Skip the rest of the IRET frame
    pop rcx     ; RIP
    add rsp, 8
    pop r11     ; RFLAGS
    pop rsp

    o64 sysret          ; Return
