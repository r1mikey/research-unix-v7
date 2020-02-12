.syntax unified
.arm
.section .text

.extern errno


@ int wait(int *status)
.global wait
.type wait,%function
wait:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    mov     r3, r0                                     @ back up the original argument (status)
    swi     #7                                         @ 7 is wait
    bcc     1f                                         @ no error, go to return value manipulation
    ldr     r2, =errno                                 @ on error, set errno and jump to return
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
    b       2f                                         @ skip to the epilogue
1:  cmp     r3, #0                                     @ did we get a status pointer?
    beq     2f                                         @ nope, skip to the epilogue
    str     r1, [r3]                                   @ store status to the output variable
2:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size wait, . - wait
