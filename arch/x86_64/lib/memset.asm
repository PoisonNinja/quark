global _ZN6String6memsetEPvim
_ZN6String6memsetEPvim:
    cld
    mov rcx, rdx
    and edx, 7
    shr rcx, 3
    movzx esi, sil
    mov rax, 0x0101010101010101
    imul rax, rsi
    rep stosq
    mov ecx, edx
    rep stosb
    mov rax, r9
    ret
