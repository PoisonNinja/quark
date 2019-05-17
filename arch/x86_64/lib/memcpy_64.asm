; Based on Linux memcpy but converted to AT&T syntax

global _ZN6libcxx6memcpyEPvPKvm:function
_ZN6libcxx6memcpyEPvPKvm:
    push rbp
    mov rbp, rsp
    cld             ; For safety
    mov rax, rdi    ; memcpy returns dest pointer
    mov rcx, rdx    ; Move count into rcx
    shr rcx, 3      ; Align size to a multiple of 8
    and rdx, 7      ; Store remaining size
    rep movsq       ; Use movsq to move 8 bytes
    mov rcx, rdx    ; Move remaining size into rcx
    rep movsb       ; Move remaining bytes
    pop rbp
    ret             ; Return
