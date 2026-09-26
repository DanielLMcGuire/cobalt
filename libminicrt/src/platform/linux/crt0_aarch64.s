// ++C CRT | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

.text
.global _start
.type _start, %function

_start:
    mov x29, #0
    ldr x0, [sp]
    add x1, sp, #8
    mov x2, sp
    and x2, x2, #-16
    mov sp, x2
    bl pluspluscBoot

1:  wfe
    b 1b
.size _start, . - _start

.section .note.GNU-stack,"",%progbits
