.syntax unified
.arm
.section .text

.extern errno
.extern _cleanup


.global exit
.type exit,%function
exit:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r0, r1, r2, r3}                           @ stash caller preserved registers
    bl      _cleanup                                   @ call atexit handlers
    pop     {r0, r1, r2, r3}                           @ restore caller preserved registers
    swi     #1                                         @ 1 is exit - exit code is in r0 - no return on success
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size exit, . - exit
