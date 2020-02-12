.syntax unified
.arm
.section .text

.extern errno

@ int alarm(int deltat)
.global alarm
.type alarm,%function
alarm:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    swi     #27                                        @ 27 is alarm, just pass r0 on...
    bcc     1f
    ldr     r2, =errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size alarm, . - alarm
