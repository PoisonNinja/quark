
extern syscall_table

global syscall_sysret_wrapper
syscall_sysret_wrapper:
;     swapgs              ; Thread struct is saved in KernelGSBase, swap it into GS
;     mov [gs:88], rsp    ; Save user RSP into thread->cpu_ctx.rsp
;     cmp rax, 57         ; Fork needs special handling, but this is expensive
;                         ; so try to avoid doing this
;     jne continue        ; Not fork, so we can skip these steps
;     mov [gs:8], r15     ; Load these registers into thread state so we can copy
;     mov [gs:16], r14
;     mov [gs:24], r13
;     mov [gs:32], r12
;     mov [gs:96], rbp
;     mov [gs:120], rbx
;     mov [gs:136], rcx   ; RIP
; continue:
;     mov rsp, [gs:176]   ; Load kernel RSP from thread->kernel_stack.
;     push qword [gs:88]  ; Push the user RSP onto the stack
;     swapgs              ; Swap back GS values. This is important because
;                         ; we may not reach the end of this function, especially
;                         ; if the system call is sys_exit. If that happens,
;                         ; future syscalls will panic because GS contains
;                         ; the wrong value
;     push rdi            ; Preserve values
;     push rsi
;     push rdx
;     push rcx
;     push r8
;     push r9
;     push r10
;     push r11
;     cmp qword [syscall_table + rax * 8], 0
;     je invalid_syscall
;     call [syscall_table + rax * 8] ; Call the system call
; return:
;     pop r11             ; Restore the registers
;     pop r10
;     pop r9
;     pop r8
;     pop rcx
;     pop rdx
;     pop rsi
;     pop rdi
;     pop rsp             ; Pop user stack back into RSP
;     o64 sysret          ; Return
; invalid_syscall:
;     mov rax, -38        ; -ENOSYS
;     jmp return
