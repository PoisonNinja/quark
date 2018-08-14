section .init
%ifdef X86_64
	pop rbp
%else
	pop ebp
%endif
	ret

section .fini
%ifdef X86_64
	pop rbp
%else
	pop ebp
%endif
	ret