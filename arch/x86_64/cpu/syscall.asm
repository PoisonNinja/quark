%include "common.inc"

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

    PUSHA

    push qword 0x1B ; DS
    push qword 0
    push qword 0

    swapgs              ; Swap back GS values. This is important because
                        ; we may not reach the end of this function, especially
                        ; if the system call is sys_exit. If that happens,
                        ; future syscalls will panic because GS contains
                        ; the wrong value

    mov rdi, rsp
    call syscall_trampoline

    pop r15
    pop r15
    pop r15 ; DS

    POPA

    add rsp, 16 ; Skip the rest of the IRET frame
    pop rcx     ; RIP
    add rsp, 8
    pop r11     ; RFLAGS
    pop rsp

    o64 sysret          ; Return
