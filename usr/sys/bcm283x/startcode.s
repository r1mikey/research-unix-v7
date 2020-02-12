@
@ Assembly code used to generate the startup stub (icode) in bcm283x_machdep.c
@
.syntax unified
.arm
.section .text

.global _start
_start:
    ldr     lr, =indirect
    swi     #0
    b       .

.p2align 2
arg0: .asciz "/etc/init"

.p2align 2
arg1:
    .word   arg0
    .word   0

.p2align 2
args:
    .word   11
    .word   arg0
    .word   arg1

.p2align 2
indirect:
    .word   args
