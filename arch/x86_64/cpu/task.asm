%include "common.inc"

; void do_task_switch(addr_t* old_stack, addr_t* new_stack, addr_t cr3);
; rdi = old stack, rsi = new stack, rdx = cr3
global do_task_switch
do_task_switch:
    pushfq
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov [rdi], rsp
    mov rsp, [rsi]

    mov cr3, rdx

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    popf
    ret
