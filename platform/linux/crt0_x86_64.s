// ++C C Runtime Library (libminicrt) | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

.text
.global _start
.type _start, @function

_start:
    xorl %ebp, %ebp
    popq %rdi
    movq %rsp, %rsi
    andq $-16, %rsp
    call pluspluscBoot

    hlt
.size _start, . - _start

.section .note.GNU-stack,"",%progbits
