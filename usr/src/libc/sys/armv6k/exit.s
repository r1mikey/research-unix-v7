.syntax unified
.arm
.section .text

.extern errno


.global _exit
.type _exit,%function
_exit:
    swi     #1
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  bx      lr
.size _exit, . - _exit
