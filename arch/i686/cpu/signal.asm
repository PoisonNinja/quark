global signal_return
signal_return:
    pop eax
    pop eax
    mov ebx, [esp]

    ; Technically not syscall compliant, but we don't care about anything
    mov eax, dword 15
    int 0x80
    ; No return
