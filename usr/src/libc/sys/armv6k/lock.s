.syntax unified
.arm
.section .text

.extern errno


@ int lock(int f)
@  - flag 0 to clear the lock, non-zero to set
.global lock
.type lock,%function
lock:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r0}                                       @ flag
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    swi     #53                                        @ 53 is lock
    movcc   r0, #0                                     @ set up the success return
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size lock, . - lock
