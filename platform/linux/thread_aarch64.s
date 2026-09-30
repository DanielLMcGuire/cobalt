// ++C C Runtime Library (libminicrt) | Platform (Linux aarch64) - thread primitives
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

.text

.global __xxc_clone
.type __xxc_clone, %function
__xxc_clone:
    and  x1, x1, #-16
    stp  x0, x3, [x1, #-16]!
    mov  x0, x2
    mov  x2, x4
    mov  x3, x5
    mov  x4, x6
    mov  x8, #220
    svc  #0
    cbnz x0, 1f
    mov  x29, #0
    ldp  x1, x0, [sp], #16
    blr  x1
    mov  x8, #93
    svc  #0
1:  ret
.size __xxc_clone, . - __xxc_clone

.global __xxc_unmapself
.type __xxc_unmapself, %function
__xxc_unmapself:
    mov  x8, #215
    svc  #0
    mov  x0, #0
    mov  x8, #93
    svc  #0
.size __xxc_unmapself, . - __xxc_unmapself

.section .note.GNU-stack,"",%progbits
