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

align 4096
page_directory:
    dd (page_table + 0x7)              ; Identity map
    times 767 dd 0
    dd (page_table + 0x7)              ; -2GB mapping
    times 255 dd 0

align 4096
page_table:
    %assign i 0
    %rep 512*25
    dd (i << 12) | 0x083
    %assign i i+1
    %endrep

NO_CPUID_STRING: db "No CPUID support!", 0

halt:
    cli
    hlt
    jmp halt

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
    jmp halt

extern asm_to_cxx_trampoline

global bootstrap
bootstrap:
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

    ; Set control register flags
    mov eax, cr0
    and eax, 0xFFFFFFFB  ; Disable coprocessor emulation
    bts eax, 2      ; Set coprocessor monitoring
    bts eax, 16     ; Enable WP for Ring 0
    mov cr0, eax
    mov eax, cr4   ; Set OSFXSR and OSXMMEXCPT
    or ax, 3 << 9
    mov cr4, eax

    ; Create long mode page table and init CR3 to
    ; point to the base of the PML4 page table
    mov eax, page_directory
    mov cr3, eax

    ; Enable Long mode, SYSCALL / SYSRET instructions, and NX bit
    mov ecx, 0xC0000080
    rdmsr
    bts eax, 11
    bts eax, 0
    wrmsr

    ; enable paging to activate long mode
    mov eax, cr0
    bts eax, 31
    mov cr0, eax

    ; Allocate a stack
    mov esp, stack + 0x4000

    call asm_to_cxx_trampoline

    jmp halt

section .bss
global stack
stack:
    resb 0x4000