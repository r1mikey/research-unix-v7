.syntax unified
.arm
.section .text

.extern errno


@ int pipe(int f[2])
.global pipe
.type pipe,%function
pipe:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    cmp     r0, #0                                     @ NULL arg is not OK
    moveq   r0, #22                                    @ EINVAL
    beq     1f                                         @ jump to error handling code
    push    {r0}                                       @ stash our out pointer
    swi     #42                                        @ 42 is pipe
    bcs     1f                                         @ syscalls return carry-clear for success and carry-set for error
    pop     {r3}                                       @ load up our arg pointer
    stmia   r3, {r0, r1}                               @ stash our return values
    mov     r0, #0                                     @ return 0 in the success case
    b       2f                                         @ jump to the epilogue
1:  ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
2:  sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size pipe, . - pipe
