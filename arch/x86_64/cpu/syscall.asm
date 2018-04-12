
extern syscall_table

global syscall_sysret_wrapper
syscall_sysret_wrapper:
    swapgs              ; TSS is saved in KernelGSBase, swap it into GS
    mov [gs:88], rsp    ; Save user RSP into TSS.rsp1
    mov rsp, [gs:176]   ; Load kernel RSP from TSS.rsp0. The cool thing is
                        ; that each thread gets its own thanks to existing code
    push qword [gs:88]  ; Push the user RSP onto the stack
    swapgs              ; Swap back GS values. This is important because
                        ; we may not reach the end of this function, especially
                        ; if the system call is sys_exit. If that happens,
                        ; future syscalls will panic because GS contains
                        ; the wrong value
    push rdi            ; Preserve values
    push rsi
    push rdx
    push rcx
    push r8
    push r9
    push r10
    push r11
    call [syscall_table + rax * 8] ; Call the system call
    pop r11             ; Restore the registers
    pop r10
    pop r9
    pop r8
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rsp             ; Pop user stack back into RSP
    o64 sysret          ; Return
