.global signal_return
signal_return:
    mov %rsp, %rdi

    # Technically not syscall compliant, but we don't care about anything
    mov $0xF, %rax
    syscall
    # No return
