// ++C C Runtime Library (libminicrt) | Platform (Linux x86_64) - thread primitives
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

.text

.global __xxc_clone
.type __xxc_clone, @function
__xxc_clone:
    andq $-16, %rsi
    subq $16, %rsi
    movq %rcx, 8(%rsi)
    movq %rdi, (%rsi)
    movq %rdx, %rdi
    movq %r8,  %rdx
    movq 8(%rsp), %r10
    movq %r9,  %r8
    movl $56, %eax
    syscall
    testq %rax, %rax
    jnz 1f
    xorl %ebp, %ebp
    popq %rax
    popq %rdi
    call *%rax
    movl %eax, %edi
    movl $60, %eax
    syscall
    hlt
1:  ret
.size __xxc_clone, . - __xxc_clone

.global __xxc_unmapself
.type __xxc_unmapself, @function
__xxc_unmapself:
    movl $11, %eax
    syscall
    xorl %edi, %edi
    movl $60, %eax
    syscall
    hlt
.size __xxc_unmapself, . - __xxc_unmapself

.section .note.GNU-stack,"",@progbits
