.global _ZN6String6memsetEPvim
_ZN6String6memsetEPvim:
    cld
    mov %rdx, %rcx
    and $7, %edx
    shr $3, %rcx
    movzx %sil, %esi
    mov $0x0101010101010101, %rax
    imul %rsi, %rax
    rep stosq
    mov %edx, %ecx
    rep stosb
    mov %r9, %rax
    ret
