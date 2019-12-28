%include "common.inc"

extern syscall_trampoline

global syscall_return
global syscall_sysret_wrapper
syscall_sysret_wrapper:
    cld

    swapgs              ; Thread struct is saved in KernelGSBase, swap it into GS
    mov [gs:80], rsp    ; Save user RSP into thread->tcontext.rsp
    mov rsp, [gs:192]   ; Load kernel RSP from thread->kernel_stack_base

    ; Build a simulated interrupt frame
    push qword 0x1B ; SS
    push qword [gs:80] ; User RSP
    push r11  ; RFLAGS
    push qword 0x23 ; CS
    push rcx  ; RIP

    push qword 0 ; Error code
    push qword 0 ; Interrupt number

    PUSHA

    push qword 0x1B ; DS

    ; Store FS
    mov rcx, 0xC0000100
    rdmsr
    shl rdx, 32
    or rdx, rax
    push rdx

    ; Store GS
    mov rcx, 0xC0000101
    rdmsr
    shl rdx, 32
    or rdx, rax
    push rdx

    swapgs              ; Swap back GS values. This is important because
                        ; we may not reach the end of this function, especially
                        ; if the system call is sys_exit. If that happens,
                        ; future syscalls will panic because GS contains
                        ; the wrong value

    mov rdi, rsp
    call syscall_trampoline

syscall_return:
    ; There isn't really a reason for system calls to be modifying FS and GS
    pop r15
    pop r15

    pop r15 ; DS

    POPA

    add rsp, 16 ; Skip the rest of the IRET frame
    pop rcx     ; RIP
    add rsp, 8  ; Skip CS
    pop r11     ; RFLAGS
    pop rsp     ; RSP

    o64 sysret          ; Return
