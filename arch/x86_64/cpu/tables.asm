global gdt_load
gdt_load:
    lgdt [rdi]
    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax        ; Load all data segment selectors
    mov es, ax
    mov fs, ax
    mov ss, ax
    ; flush the CS segment with iretq
    mov     rcx, qword .reloadcs
    mov     rsi, rsp

    push    rax             ; new SS
    push    rsi             ; new RSP
    push    2               ; new FLAGS
    push    0x8             ; new CS
    push    rcx             ; new RIP
    iretq
.reloadcs:
	ret
