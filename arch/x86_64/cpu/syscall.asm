
extern syscall_table

global syscall_sysret_wrapper
syscall_sysret_wrapper:
    swapgs              ; TSS is saved in KernelGSBase, swap it into GS
    mov [gs:12], rsp    ; Save user RSP into TSS.rsp1
    mov rsp, [gs:4]     ; Load kernel RSP from TSS.rsp0. The cool thing is
                        ; that each thread gets its own thanks to existing code
    push qword [gs:12]  ; Push the user RSP onto the stack
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

global syscall_init
syscall_init:
    ; STAR - Set segment registers
    mov ecx, 0xC0000081
    mov edx, 0x00130008 ; CS = 0x23 | CS = 0x08
                        ; There is an interesting quirk with sysret in long mode
                        ; syscall and sysret (32-bit) load CS directly, but
                        ; sysret in 64-bit adds 16. It also forces us to have
                        ; the data descriptor for ring 3 before the code
                        ; descriptor
    xor eax, eax        ; Zero out EAX
    wrmsr

    ; LSTAR
    mov ecx, 0xC0000082
    mov rdx, syscall_sysret_wrapper ; Load address of syscall entry point
    mov rax, rdx                    ; EAX stores lower 32 bits
    shr rdx, 32                     ; EDX stores upper 32 bits
    wrmsr

    ; Set RFLAGS mask
    mov ecx, 0xC0000084
    mov eax, 0x200 ; Clear IF because we don't want interrupts during syscalls
    wrmsr
    ret
