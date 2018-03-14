bits 64
section .text

extern asm_to_cxx_trampoline

global bootstrap64
bootstrap64:
    ; From here on out, we are running instructions
    ; within the higher half (0xffffffff80000000 ... )

    ; Load in the segment descriptor
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Allocate a stack
    mov rsp, stack + 0x4000

    ; update the multiboot struct to point to a
    ; virtual address
    add rsi, 0xFFFFFFFF80000000

    ; push the parameters (just in case)
    push rsi
    push rdi

    ; Unmap the lower half
    mov rax, cr3
    mov [rax], dword 0x0
    mov cr3, rax

    ; Call x86_64 initialization function
    call asm_to_cxx_trampoline

    ; We shouldn't reach here, but if we do, loop forever
    jmp halt64

halt64:
    cli
    hlt
    jmp halt64

section .bss
global stack
stack:
    resb 0x4000
