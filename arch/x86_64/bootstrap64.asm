bits 64
section .text

extern x86_64_init

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

    ; Call x86_64 initialization function
    call x86_64_init

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
