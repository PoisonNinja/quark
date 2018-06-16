global gdt_load
gdt_load:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax        ; Load all data segment selectors
    mov es, ax
    mov ss, ax

    mov ax, 0x33
    mov fs, ax
    mov ax, 0x3B
    mov gs, ax

    jmp 0x8:.reloadcs

.reloadcs:
    ret

global tss_load
tss_load:
    mov ax, (0x28 | 3)
    ltr ax
    ret

%macro ISR_NOERROR_CODE 1
  [GLOBAL isr%1]
  isr%1:
    push dword 0
    push dword %1
    jmp interrupt_common_stub
%endmacro

%macro ISR_ERROR_CODE 1
  [GLOBAL isr%1]
  isr%1:
    push dword %1
    jmp interrupt_common_stub
%endmacro

%macro IRQ 2
  [GLOBAL irq%1]
  irq%1:
    push dword 0
    push dword %2
    jmp interrupt_common_stub
%endmacro

ISR_NOERROR_CODE 0
ISR_NOERROR_CODE 1
ISR_NOERROR_CODE 2
ISR_NOERROR_CODE 3
ISR_NOERROR_CODE 4
ISR_NOERROR_CODE 5
ISR_NOERROR_CODE 6
ISR_NOERROR_CODE 7
ISR_ERROR_CODE 8
ISR_NOERROR_CODE 9
ISR_ERROR_CODE 10
ISR_ERROR_CODE 11
ISR_ERROR_CODE 12
ISR_ERROR_CODE 13
ISR_ERROR_CODE 14
ISR_NOERROR_CODE 15 ; Reserved
ISR_NOERROR_CODE 16
ISR_ERROR_CODE 17
ISR_NOERROR_CODE 18
ISR_NOERROR_CODE 19
ISR_NOERROR_CODE 20
ISR_NOERROR_CODE 21 ; Reserved
ISR_NOERROR_CODE 22 ; Reserved
ISR_NOERROR_CODE 23 ; Reserved
ISR_NOERROR_CODE 24 ; Reserved
ISR_NOERROR_CODE 25 ; Reserved
ISR_NOERROR_CODE 26 ; Reserved
ISR_NOERROR_CODE 27 ; Reserved
ISR_NOERROR_CODE 28 ; Reserved
ISR_NOERROR_CODE 29 ; Reserved
ISR_ERROR_CODE 30
ISR_NOERROR_CODE 31

ISR_NOERROR_CODE 128 ; System call vector
ISR_NOERROR_CODE 129 ; Yield

; IRQs
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; Portions of the following code are taken from Sortix by Jonas 'Sortie' Termansen
;
; Sortix is licensed under the ISC license, and a copy can be found online
; at https://gitlab.com/sortix/sortix/blob/master/LICENSE
;
; The code taken from Sortix is mostly about the intrapriv_* stuff, and is
; nearly verbatim copied except that it is translated from AT&T to Intel
; syntax

extern arch_handler
interrupt_common_stub:
    cld
    test dword [esp + 12], 0x3
    jz intrapriv_entry

intrapriv_entry_ret:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov ss, ax

    push esp
    call arch_handler
    add esp, 4

interrupt_return:
    pop eax
    mov ds, ax
    popa
    add esp, 8

    test dword [esp + 4], 0x3
    jz intrapriv_exit

intrapriv_exit_ret:
    iret

intrapriv_entry:
    mov [esp - 0xC], eax
    mov eax, [esp]
    mov [esp - 0x8], eax
    mov eax, [esp + 0x4]
    mov [esp - 0x4], eax
    mov eax, [esp + 0x8]
    mov [esp], eax
    mov eax, [esp + 0xc]
    mov [esp + 0x4], eax
    mov eax, [esp + 0x10]
    mov [esp + 0x8], eax
    mov eax, esp
    add eax, 0x14
    mov [esp + 0xC], eax
    mov eax, ss
    mov [esp + 0x10], eax
    mov eax, [esp - 0xC]
    sub esp, 0x8
    jmp intrapriv_entry_ret

intrapriv_exit:
    mov [esp - 0x4],eax
    mov [esp - 0x8],ebx
    mov [esp - 0xC],ecx
    mov ebx, [esp + 0xC]
    sub ebx, 0xC
    mov cx, [esp + 0x10]
    mov eax, [esp - 0x4]
    mov [ebx - 0xc],eax
    mov eax, [esp - 0x8]
    mov [ebx - 0x10],eax
    mov eax, [esp - 0xc]
    mov [ebx - 0x14],eax
    mov eax, [esp + 0x8]
    mov [ebx + 0x8],eax
    mov eax, [esp + 0x4]
    mov [ebx + 0x4],eax
    mov eax, [esp]
    mov [ebx],eax
    mov ss,ecx
    mov esp,ebx
    mov eax, [esp - 0xC]
    mov ebx, [esp - 0x10]
    mov ecx, [esp - 0x14]
    jmp  intrapriv_exit_ret

global idt_load
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

global load_register_state
load_register_state:
    mov esp, [esp + 4]
    jmp interrupt_return
