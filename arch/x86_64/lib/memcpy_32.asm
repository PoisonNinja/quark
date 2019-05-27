; Based on Linux memcpy but converted to AT&T syntax

global _ZN6libcxx6memcpyEPvPKvm:function
_ZN6libcxx6memcpyEPvPKvm:
    push ebp              ; Base pointer (function prologue)
    mov ebp, esp
    push esi              ; Callee-saved
    push edi
    cld                   ; For safety
    mov eax, [esp + 16]   ; memcpy returns dest pointer
    mov edi, [esp + 16]   ; Destination
    mov esi, [esp + 20]   ; Source
    mov edx, [esp + 24]   ; Move count into rcx
    mov ecx, edx
    shr ecx, 2            ; Align size to a multiple of 4
    and edx, 3            ; Store remaining size
    rep movsd             ; Use movsd to move 4 bytes
    mov ecx, edx          ; Move remaining size into rcx
    rep movsb             ; Move remaining bytes
    pop edi               ; Restore callee-saved
    pop esi
    pop ebp               ; Function epilogue
    ret                   ; Return
