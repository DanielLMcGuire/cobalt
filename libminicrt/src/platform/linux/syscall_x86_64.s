// ++C CRT | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

.text
.global __syscall0
.type __syscall0, @function
__syscall0:
    movq %rdi, %rax
    syscall
    ret
.size __syscall0, . - __syscall0

.global __syscall1
.type __syscall1, @function
__syscall1:
    movq %rdi, %rax
    movq %rsi, %rdi
    syscall
    ret
.size __syscall1, . - __syscall1

.global __syscall2
.type __syscall2, @function
__syscall2:
    movq %rdi, %rax
    movq %rsi, %rdi
    movq %rdx, %rsi
    syscall
    ret
.size __syscall2, . - __syscall2

.global __syscall3
.type __syscall3, @function
__syscall3:
    movq %rdi, %rax
    movq %rsi, %rdi
    movq %rdx, %rsi
    movq %rcx, %rdx
    syscall
    ret
.size __syscall3, . - __syscall3

.global __syscall4
.type __syscall4, @function
__syscall4:
    movq %rdi, %rax
    movq %rsi, %rdi
    movq %rdx, %rsi
    movq %rcx, %rdx
    movq %r8,  %r10
    syscall
    ret
.size __syscall4, . - __syscall4

.global __syscall6
.type __syscall6, @function
__syscall6:
    movq %rdi, %rax
    movq %rsi, %rdi
    movq %rdx, %rsi
    movq %rcx, %rdx
    movq %r8,  %r10
    movq %r9,  %r8
    movq 8(%rsp), %r9
    syscall
    ret
.size __syscall6, . - __syscall6

.global __restore_rt
.hidden __restore_rt
.type __restore_rt, @function
__restore_rt:
    movq $15, %rax
    syscall
.size __restore_rt, . - __restore_rt
