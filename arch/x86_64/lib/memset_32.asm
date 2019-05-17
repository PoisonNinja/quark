global _ZN6libcxx6memsetEPvim:function
_ZN6libcxx6memsetEPvim:
    push ebp
    mov ebp, esp
    push esi
    push edi
    push dword [esp + 16]      ; Store destination as rep destroys edi
    cld                        ; For safety
    mov edx, [esp + 28]        ; Count
    mov edi, [esp + 20]        ; Destination
    mov ecx, edx               ; rep uses ecx
    shr ecx, 2                 ; Divide by 4 to align to 4 bytes
    and edx, 3                 ; Store remainder in edx
    movzx esi, byte [esp + 24] ; Character
    mov eax, 0x01010101
    imul eax, esi              ; Extend the character to the entire register
    rep stosd                  ; Copy 4 bytes at a time
    mov ecx, edx               ; Move remainder into counter
    rep stosb                  ; Copy remaining bytes
    pop eax                    ; Pop destination into return value
    pop edi                    ; Restore used registers
    pop esi
    pop ebp
    ret
