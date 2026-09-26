// ++C CRT | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

.text
.global __syscall0
.type __syscall0, %function
__syscall0:
    mov x8, x0
    svc #0
    ret
.size __syscall0, . - __syscall0

.global __syscall1
.type __syscall1, %function
__syscall1:
    mov x8, x0
    mov x0, x1
    svc #0
    ret
.size __syscall1, . - __syscall1

.global __syscall2
.type __syscall2, %function
__syscall2:
    mov x8, x0
    mov x0, x1
    mov x1, x2
    svc #0
    ret
.size __syscall2, . - __syscall2

.global __syscall3
.type __syscall3, %function
__syscall3:
    mov x8, x0
    mov x0, x1
    mov x1, x2
    mov x2, x3
    svc #0
    ret
.size __syscall3, . - __syscall3

.global __syscall4
.type __syscall4, %function
__syscall4:
    mov x8, x0
    mov x0, x1
    mov x1, x2
    mov x2, x3
    mov x3, x4
    svc #0
    ret
.size __syscall4, . - __syscall4

.global __syscall6
.type __syscall6, %function
__syscall6:
    mov x8, x0
    mov x0, x1
    mov x1, x2
    mov x2, x3
    mov x3, x4
    mov x4, x5
    mov x5, x6
    svc #0
    ret
.size __syscall6, . - __syscall6
