# Based on Linux memcpy but converted to AT&T syntax

.global _ZN6String6memcpyEPvPKvm
_ZN6String6memcpyEPvPKvm:
    cld             # For safety
    mov %rdi, %rax  # memcpy returns dest pointer
    mov %rdx, %rcx  # Move count into rcx
    shr $3, %rcx    # Align size to a multiple of 8
    and $7, %rdx    # Store remaining size
    rep movsq       # Use movsq to move 8 bytes
    mov %rdx, %rcx  # Move remaining size into rcx
    rep movsb       # Move remaining bytes
    ret             # Return
