// ++C CRT | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

.text
.global syscall
.type syscall, @function
syscall:
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %ebx

    movl 20(%esp), %eax
    movl 24(%esp), %ebx
    movl 28(%esp), %ecx
    movl 32(%esp), %edx
    movl 36(%esp), %esi
    movl 40(%esp), %edi
    movl 44(%esp), %ebp
    int $0x80

    popl %ebx
    popl %esi
    popl %edi
    popl %ebp
    ret
.size syscall, . - syscall

.global __restore_rt
.hidden __restore_rt
.type __restore_rt, @function
__restore_rt:
    movl $173, %eax
    int $0x80
.size __restore_rt, . - __restore_rt
