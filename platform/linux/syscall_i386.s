// ++C CRT | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

.text
.global __syscall0
.type __syscall0, @function
__syscall0:
    movl 4(%esp), %eax
    int $0x80
    ret
.size __syscall0, . - __syscall0

.global __syscall1
.type __syscall1, @function
__syscall1:
    pushl %ebx
    movl 8(%esp),  %eax
    movl 12(%esp), %ebx
    int $0x80
    popl %ebx
    ret
.size __syscall1, . - __syscall1

.global __syscall2
.type __syscall2, @function
__syscall2:
    pushl %ebx
    movl 8(%esp),  %eax
    movl 12(%esp), %ebx
    movl 16(%esp), %ecx
    int $0x80
    popl %ebx
    ret
.size __syscall2, . - __syscall2

.global __syscall3
.type __syscall3, @function
__syscall3:
    pushl %ebx
    movl 8(%esp),  %eax
    movl 12(%esp), %ebx
    movl 16(%esp), %ecx 
    movl 20(%esp), %edx
    int $0x80
    popl %ebx
    ret
.size __syscall3, . - __syscall3

.global __syscall4
.type __syscall4, @function
__syscall4:
    pushl %esi
    pushl %ebx
    movl 12(%esp), %eax
    movl 16(%esp), %ebx
    movl 20(%esp), %ecx
    movl 24(%esp), %edx
    movl 28(%esp), %esi
    int $0x80
    popl %ebx
    popl %esi
    ret
.size __syscall4, . - __syscall4

.global __syscall6
.type __syscall6, @function
__syscall6:
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
.size __syscall6, . - __syscall6

.global __restore_rt
.hidden __restore_rt
.type __restore_rt, @function
__restore_rt:
    movl $173, %eax
    int $0x80
.size __restore_rt, . - __restore_rt
