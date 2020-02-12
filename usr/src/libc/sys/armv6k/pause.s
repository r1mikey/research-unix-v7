.syntax unified
.arm
.section .text

.extern errno


@ int pause(void)
.global pause
.type pause,%function
pause:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    swi     #29                                        @ 29 is pause
    bcc     1f
    ldr     r2, =errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size pause, . - pause
