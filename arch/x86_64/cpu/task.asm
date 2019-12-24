%include "common.inc"

; void do_task_switch(addr_t* old_stack, addr_t* new_stack);
; rdi = old stack, rsi = new stack
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

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    popf
    ret
