// ++C CRT | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

.text
.global syscall
.type syscall, @function
syscall:
    movq %rdi, %rax
    movq %rsi, %rdi
    movq %rdx, %rsi
    movq %rcx, %rdx 
    movq %r8,  %r10
    movq %r9,  %r8
    movq 8(%rsp), %r9
    syscall
    ret
.size syscall, . - syscall

.global __restore_rt
.hidden __restore_rt
.type __restore_rt, @function
__restore_rt:
    movq $15, %rax
    syscall
.size __restore_rt, . - __restore_rt

.section .note.GNU-stack,"",@progbits
