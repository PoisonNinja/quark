MULTIBOOT2_MAGIC                equ    0xE85250D6       ; Magic number
MULTIBOOT2_ARCHITECTURE         equ    0                ; 32-bit i386
MULTIBOOT2_SIZE                 equ    (multiboot2_end - multiboot2_start)
MULTIBOOT2_CHECKSUM             equ    0x100000000 - (MULTIBOOT2_MAGIC + MULTIBOOT2_ARCHITECTURE + MULTIBOOT2_SIZE)

bits 32
section .bootstrap
align 8

multiboot2_start:
dd MULTIBOOT2_MAGIC
dd MULTIBOOT2_ARCHITECTURE
dd MULTIBOOT2_SIZE
dd MULTIBOOT2_CHECKSUM

dw 0
dw 0
dd 8
multiboot2_end:

NO_CPUID_STRING: db "No CPUID support!", 0
NO_LONG_MODE: db "No long mode support! Use a 32-bit build!", 0
NO_SSE_STRING: db "No SSE2 support! Use a 32-bit build!", 0

extern bootstrap64

align 4096
gdt64:                           ; Global Descriptor Table (64-bit).
    .null: equ $ - gdt64         ; The null descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0                         ; Access.
    db 0                         ; Granularity.
    db 0                         ; Base (high).
    .code: equ $ - gdt64         ; The code descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10011010b                 ; Access (exec/read).
    db 00100000b                 ; Granularity.
    db 0                         ; Base (high).
    .data: equ $ - gdt64         ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010010b                 ; Access (read/write).
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .pointer:                    ; The GDT-pointer.
    dw $ - gdt64 - 1             ; Limit.
    dq gdt64                     ; Base.

align 4096
pml4_base:
    dq (pml3_base + 0x7)
    times 510 dq 0
    dq (pml3_base + 0x7)

align 4096
pml3_base:
    dq (pml2_base + 0x7)
    times 509 dq 0
    dq (pml2_base + 0x7)
    dq 0

align 4096
pml2_base:
    %assign i 0
    %rep 25
    dq (pml1_base + i + 0x7)
    %assign i i+4096
    %endrep

    times (512-25) dq 0

align 4096
; 15 tables are described here
; this maps 40 MB from address 0x0
; to an identity mapping
pml1_base:
    %assign i 0
    %rep 512*25
    dq (i << 12) | 0x087
    %assign i i+1
    %endrep

halt32:
    cli
    hlt
    jmp halt32

print_string:
    pusha
    mov edx, 0xB8000

.print_string_loop:
    mov al, [ebx]
    mov ah, 0x0F

    cmp al, 0
    je .print_string_cleanup

    mov [edx], ax
    inc ebx
    add edx, 2

    jmp .print_string_loop

.print_string_cleanup:
    popa
    ret

wipe_screen:
    pusha
    mov edx, 0xB8000

.wipe_screen_loop:
    mov ax, 0
    mov [edx], ax

    cmp edx, 0xB8FA0
    jge .wipe_screen_ret

    add edx, 2
    jmp .wipe_screen_loop

.wipe_screen_ret:
    popa
    ret

noCPUID:
    mov ebx, NO_CPUID_STRING
    call print_string
    jmp halt32

noLongMode:
    mov ebx, NO_LONG_MODE
    call print_string
    jmp halt32

noSSE2:
    mov ebx, NO_SSE_STRING
    call print_string
    jmp halt32

global bootstrap32
bootstrap32:
    ; Back up the multiboot data in the registers (magic, info struct)
    mov esi, ebx
    mov edi, eax

    ; Why don't we use the nice SSE2 optimized memset implementation in asm?
    ; At this point, we don't know whether this is a SSE2 compatible or even
    ; 64-bit computer, and attempting to use those instructions on a computer
    ; without SSE2 or long mode would cause a crash
    call wipe_screen

    pushfd                               ;Save EFLAGS
    pushfd                               ;Store EFLAGS
    xor dword [esp],0x00200000           ;Invert the ID bit in stored EFLAGS
    popfd                                ;Load stored EFLAGS (with ID bit inverted)
    pushfd                               ;Store EFLAGS again (ID bit may or may not be inverted)
    pop eax                              ;eax = modified EFLAGS (ID bit may or may not be inverted)
    xor eax,[esp]                        ;eax = whichever bits were changed
    popfd                                ;Restore original EFLAGS
    and eax,0x00200000                   ;eax = zero if ID bit can't be changed, else non-zero
    cmp eax, 0
    je noCPUID

    mov eax, 0x00000001                  ; Long mode CPUs should have SSE, but check just in case
    cpuid
    test edx, 0x4000000
    jz noSSE2

    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001                  ; Compare the A-register with 0x80000001.
    jb noLongMode                        ; It is less, there is no long mode.

    mov eax, 0x80000001                  ; Set the A-register to 0x80000001.
    cpuid                                ; CPU identification.
    test edx, 1 << 29                    ; Test if the LM-bit, which is bit 29, is set in the D-register.
    jz noLongMode                        ; They aren't, there is no long mode.

    ; enable 64-bit page translation table entries
    ; by setting CR4.PAE = 1.
    ;
    ; Paging is not enabled until long mode.
    mov eax, cr4
    bts eax, 5
    mov cr4, eax

    ; Enable SSE instructions. Without this, we would crash as soon as we used
    ; them (e.g. memset, memcpy, strcpy)
    mov eax, cr0
    and ax, 0xFFFB
    or ax, 0x2
    mov cr0, eax
    mov eax, cr4
    or ax, 3 << 9
    mov cr4, eax

    ; Create long mode page table and init CR3 to
    ; point to the base of the PML4 page table
    mov eax, pml4_base
    mov cr3, eax

    ; Enable Long mode, SYSCALL / SYSRET instructions, and NX bit
    mov ecx, 0xC0000080
    rdmsr
    bts eax, 11
    bts eax, 8
    bts eax, 0
    wrmsr

    ; enable paging to activate long mode
    mov eax, cr0
    bts eax, 31
    mov cr0, eax

    lgdt [gdt64.pointer]         ; Load the 64-bit global descriptor table.

    jmp 0x08:(trampoline)

bits 64
trampoline:
    mov rcx, bootstrap64
    jmp rcx
