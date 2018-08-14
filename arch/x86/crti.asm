section .init
global _init
_init:
%ifdef X86_64
	push rbp
	mov rbp, rsp
%else
	push ebp
	mov ebp, esp
%endif

section .fini
global _fini
_fini:
%ifdef X86_64
	push rbp
	mov rbp, rsp
%else
	push ebp
	mov ebp, esp
%endif
