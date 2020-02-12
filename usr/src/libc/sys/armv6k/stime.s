.syntax unified
.arm
.section .text

.extern errno


@ ... where time_t is a long in userland, so 32 bit
@ we just lie and send through an r1 value of #0
@ int stime(time_t *t)
@ TODO: make time_t 64bit (much later)
@ TODO: make time_t a 32 bit value soonish
.global stime
.type stime,%function
stime:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    ldr     r0, [r0]                                   @ load up the pointed-to value
    mov     r1, #0                                     @ for now we lie and send through a 0 value for the second part of time_t
    swi     #25                                        @ 25 is stime
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size stime, . - stime
