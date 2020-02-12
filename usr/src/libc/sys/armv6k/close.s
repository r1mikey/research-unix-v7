.syntax unified
.arm
.section .text

.extern errno


.global close
.type close,%function
close:
    mov     r3, lr
    swi     #6
    movcc   r0, #0                                     @ syscall doesn't return anything meaningful
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  mov     lr, r3
    bx      lr
.size close, . - close
