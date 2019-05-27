global signal_return
signal_return:
    mov rdi, rsp

    ; Technically not syscall compliant, but we don't care about anything
    mov rax, qword 15
    syscall
    ; No return
