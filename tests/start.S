.section .text.start,"ax"
.balign 4
.global _start
_start:
    mov.l .Lstack,r15
    mov.l .Lmain,r0
    jsr @r0
    nop
.global dcc_test_done
dcc_test_done:
    bra dcc_test_done
    nop
.balign 4
.Lstack:
    .long __stack_top
.Lmain:
    .long _dcc_selftest
