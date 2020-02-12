.syntax unified
.arm
.section .text

.extern errno


.globl open
.type open,%function
open:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    sub     sp, sp, #16                                @ create scratch space for args
    str     r1, [fp, #-4]                              @ flags
    str     r0, [fp, #-8]                              @ path
    mov     r1, #5
    str     r1, [fp, #-12]                             @ syscall number
    sub     r1, fp, #12                                @ args struct pointer {syscall, arg0, arg1}
    str     r1, [fp, #-16]                             @ pointer to args
    sub     lr, fp, #16                                @ pointer to indirect args is kept in lr
    swi     #0
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size open, . - open
