global signal_return
signal_return:
%ifdef X86_64
    mov rdi, rsp

    ; Technically not syscall compliant, but we don't care about anything
    mov rax, qword 15
    syscall
    ; No return
%else
    pop eax
    pop eax
    mov ebx, [esp]

    ; Technically not syscall compliant, but we don't care about anything
    mov eax, dword 15
    int 0x80
    ; No return
%endif
