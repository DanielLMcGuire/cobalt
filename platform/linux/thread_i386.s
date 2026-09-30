// ++C C Runtime Library (libminicrt) | Platform (Linux i386) - thread primitives
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

.text

.global __xxc_clone
.type __xxc_clone, @function
__xxc_clone:
    pushl %ebx
    pushl %esi
    pushl %edi
    pushl %ebp
    movl 24(%esp), %ecx
    andl $-16, %ecx
    subl $16, %ecx
    movl 32(%esp), %eax
    movl %eax, 4(%ecx)
    movl 20(%esp), %eax
    movl %eax, (%ecx)
    movl 28(%esp), %ebx
    movl 36(%esp), %edx
    movl 40(%esp), %esi
    movl 44(%esp), %edi
    movl $120, %eax
    int $0x80
    testl %eax, %eax
    jnz 1f
    xorl %ebp, %ebp
    popl %eax
    popl %ecx
    subl $12, %esp
    pushl %ecx
    call *%eax
    movl %eax, %ebx
    movl $1, %eax
    int $0x80
    hlt
1:  popl %ebp
    popl %edi
    popl %esi
    popl %ebx
    ret
.size __xxc_clone, . - __xxc_clone

.global __xxc_unmapself
.type __xxc_unmapself, @function
__xxc_unmapself:
    movl 4(%esp), %ebx
    movl 8(%esp), %ecx
    movl $91, %eax
    int $0x80
    xorl %ebx, %ebx
    movl $1, %eax
    int $0x80
    hlt
.size __xxc_unmapself, . - __xxc_unmapself

.section .note.GNU-stack,"",@progbits
