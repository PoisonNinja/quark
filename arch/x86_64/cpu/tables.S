.global gdt_load
gdt_load:
    lgdt 0(%rdi)
    mov $0x10, %ax  # 0x10 is the offset in the GDT to our data segment
    mov %ax, %ds      # Load all data segment selectors
    mov %ax, %es
    mov %ax, %ss

    # flush the CS segment with iretq
    mov $.reloadcs, %rcx
    mov %rsp, %rsi

    push %rax             # new SS
    push %rsi             # new RSP
    pushq $2              # new FLAGS
    pushq $0x8            # new CS
    push %rcx             # new RIP
    iretq
.reloadcs:
    ret

.global tss_load
tss_load:
    mov $(0x28 | 3), %ax
    ltr %ax

    # Set FS and GS to enable writing to them using MSRs
    mov $0x1B, %ax
    mov %ax, %fs
    mov %ax, %gs

    ret

.global gs_load
gs_load:
    # KernelGSBase - The MSR that swapgs loads from
    mov $0xC0000102, %ecx
    mov %rdi, %rax        # EAX stores lower 32 bits
    mov %rdi, %rdx        # EDX stores upper 32 bits
    shr $32, %rdx
    wrmsr
    ret

.macro ISR_NOERROR_CODE int_no
  .global isr\int_no
  isr\int_no:
    pushq $0
    pushq $\int_no
    jmp interrupt_common_stub
.endm

.macro ISR_ERROR_CODE int_no
  .global isr\int_no
  isr\int_no:
    pushq $\int_no
    jmp interrupt_common_stub
.endm

.macro IRQ irq_no, int_no
  .global irq\irq_no
  irq\irq_no:
    pushq $0
    pushq $\int_no
    jmp interrupt_common_stub
.endm

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
ISR_NOERROR_CODE 15 # Reserved
ISR_NOERROR_CODE 16
ISR_ERROR_CODE 17
ISR_NOERROR_CODE 18
ISR_NOERROR_CODE 19
ISR_NOERROR_CODE 20
ISR_NOERROR_CODE 21 # Reserved
ISR_NOERROR_CODE 22 # Reserved
ISR_NOERROR_CODE 23 # Reserved
ISR_NOERROR_CODE 24 # Reserved
ISR_NOERROR_CODE 25 # Reserved
ISR_NOERROR_CODE 26 # Reserved
ISR_NOERROR_CODE 27 # Reserved
ISR_NOERROR_CODE 28 # Reserved
ISR_NOERROR_CODE 29 # Reserved
ISR_ERROR_CODE 30
ISR_NOERROR_CODE 31

ISR_NOERROR_CODE 128 # System call vector
ISR_NOERROR_CODE 129 # Yield

# IRQs
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

.extern arch_handler
interrupt_common_stub:
    cld

    push %rax     # save current rax
    push %rbx     # save current rbx
    push %rcx     # save current rcx
    push %rdx     # save current rdx
    push %rbp     # save current rbp
    push %rdi     # save current rdi
    push %rsi     # save current rsi
    push %r8      # save current r8
    push %r9      # save current r9
    push %r10     # save current r10
    push %r11     # save current r11
    push %r12     # save current r12
    push %r13     # save current r13
    push %r14     # save current r14
    push %r15     # save current r15

    # Store DS
    xor %rax, %rax
    mov %ds, %ax
    push %rax

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es

    # Store FS
    mov $0xC0000100, %rcx
    rdmsr
    shl $32, %rdx
    or %rax, %rdx
    push %rdx

    # Store GS
    mov $0xC0000101, %rcx
    rdmsr
    shl $32, %rdx
    or %rax, %rdx
    push %rdx

    mov %rsp, %rdi
    call arch_handler

interrupt_return:
    pop %rdx
    mov %edx, %eax
    shr $32, %rdx
    mov $0xC0000101, %rcx
    wrmsr

    pop %rdx
    mov %edx, %eax
    shr $32, %rdx
    mov $0xC0000100, %rcx
    wrmsr

    pop %rax
    mov %ax, %ds
    mov %ax, %es

    pop %r15     # restore current r15
    pop %r14     # restore current r14
    pop %r13     # restore current r13
    pop %r12     # restore current r12
    pop %r11     # restore current r11
    pop %r10     # restore current r10
    pop %r9      # restore current r9
    pop %r8      # restore current r8
    pop %rsi     # restore current rsi
    pop %rdi     # restore current rdi
    pop %rbp     # restore current rbp
    pop %rdx     # restore current rdx
    pop %rcx     # restore current rcx
    pop %rbx     # restore current rbx
    pop %rax     # restore current rax

    add $16, %rsp

    iretq

.global idt_load
idt_load:
    lidt (%rdi)
    ret

.global load_register_state
load_register_state:
    mov %rdi, %rsp
    jmp interrupt_return
